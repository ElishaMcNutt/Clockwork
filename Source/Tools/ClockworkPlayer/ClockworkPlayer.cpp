#include <Clockwork/Clockwork.h>

#include <Clockwork/Engine/Engine.h>
#include <Clockwork/IO/FileSystem.h>
#include <Clockwork/IO/Log.h>
#include <Clockwork/Core/Main.h>
#include <Clockwork/Core/ProcessUtils.h>
#include <Clockwork/Resource/ResourceCache.h>
#include <Clockwork/Resource/ResourceEvents.h>

#ifdef CLOCKWORK_ANGELSCRIPT
#include <Clockwork/Script/ScriptFile.h>
#include <Clockwork/Script/Script.h>
#endif

#ifdef CLOCKWORK_LUA
#include <Clockwork/LuaScript/LuaScript.h>
#endif

#include "ClockworkPlayer.h"

#include <Clockwork/DebugNew.h>

DEFINE_APPLICATION_MAIN(ClockworkPlayer);

ClockworkPlayer::ClockworkPlayer(Context* context) :
    Application(context)
{
}

void ClockworkPlayer::Setup()
{
    FileSystem* filesystem = GetSubsystem<FileSystem>();

    // Read command line from a file if no arguments given. This is primarily intended for mobile platforms.
    // Note that the command file name uses a hardcoded path that does not utilize the resource system
    // properly (including resource path prefix), as the resource system is not yet initialized at this point
    const String commandFileName = filesystem->GetProgramDir() + "Data/CommandLine.txt";
    if (GetArguments().Empty() && filesystem->FileExists(commandFileName))
    {
        SharedPtr<File> commandFile(new File(context_, commandFileName));
        String commandLine = commandFile->ReadLine();
        commandFile->Close();
        ParseArguments(commandLine, false);
        // Reparse engine startup parameters now
        engineParameters_ = Engine::ParseParameters(GetArguments());
    }

    // Check for script file name
    const Vector<String>& arguments = GetArguments();
    String scriptFileName;
    if (arguments.Size() && arguments[0][0] != '-')
        scriptFileName_ = GetInternalPath(arguments[0]);

    // Show usage if not found
    if (scriptFileName_.Empty())
    {
        ErrorExit("Usage: ClockworkPlayer <scriptfile> [options]\n\n"
            "The script file should implement the function void Start() for initializing the "
            "application and subscribing to all necessary events, such as the frame update.\n"
            #ifndef WIN32
            "\nCommand line options:\n"
            "-x <res>     Horizontal resolution\n"
            "-y <res>     Vertical resolution\n"
            "-m <level>   Enable hardware multisampling\n"
            "-v           Enable vertical sync\n"
            "-t           Enable triple buffering\n"
            "-w           Start in windowed mode\n"
            "-s           Enable resizing when in windowed mode\n"
            "-q           Enable quiet mode which does not log to standard output stream\n"
            "-b <length>  Sound buffer length in milliseconds\n"
            "-r <freq>    Sound mixing frequency in Hz\n"
            "-p <paths>   Resource path(s) to use, separated by semicolons\n"
            "-ap <paths>  Autoload resource path(s) to use, seperated by semicolons\n"
            "-log <level> Change the log level, valid 'level' values are 'debug', 'info', 'warning', 'error'\n"
            "-ds <file>   Dump used shader variations to a file for precaching\n"
            "-mq <level>  Material quality level, default 2 (high)\n"
            "-tq <level>  Texture quality level, default 2 (high)\n"
            "-tf <level>  Texture filter mode, default 2 (trilinear)\n"
            "-af <level>  Texture anisotropy level, default 4. Also sets anisotropic filter mode\n"
            "-gl2         Force OpenGL 2 use even if OpenGL 3 is available\n"
            "-flushgpu    Flush GPU command queue each frame. Effective only on Direct3D\n"
            "-borderless  Borderless window mode\n"
            "-headless    Headless mode. No application window will be created\n"
            "-landscape   Use landscape orientations (iOS only, default)\n"
            "-portrait    Use portrait orientations (iOS only)\n"
            "-prepass     Use light pre-pass rendering\n"
            "-deferred    Use deferred rendering\n"
            "-renderpath <name> Use the named renderpath (must enter full resource name)\n"
            "-lqshadows   Use low-quality (1-sample) shadow filtering\n"
            "-noshadows   Disable shadow rendering\n"
            "-nolimit     Disable frame limiter\n"
            "-nothreads   Disable worker threads\n"
            "-nosound     Disable sound output\n"
            "-noip        Disable sound mixing interpolation\n"
            "-touch       Touch emulation on desktop platform\n"
            #endif
        );
    }
    else
    {
        // Use the script file name as the base name for the log file
        engineParameters_["LogName"] = filesystem->GetAppPreferencesDir("clockwork3d", "logs") + GetFileNameAndExtension(scriptFileName_) + ".log";
    }
}

void ClockworkPlayer::Start()
{
    String extension = GetExtension(scriptFileName_);
    if (extension != ".lua" && extension != ".luc")
    {
#ifdef CLOCKWORK_ANGELSCRIPT
        // Instantiate and register the AngelScript subsystem
        context_->RegisterSubsystem(new Script(context_));

        // Hold a shared pointer to the script file to make sure it is not unloaded during runtime
        scriptFile_ = GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(scriptFileName_);

        /// \hack If we are running the editor, also instantiate Lua subsystem to enable editing Lua ScriptInstances
#ifdef CLOCKWORK_LUA
        if (scriptFileName_.Contains("Editor.as", false))
            context_->RegisterSubsystem(new LuaScript(context_));
#endif
        // If script loading is successful, proceed to main loop
        if (scriptFile_ && scriptFile_->Execute("void Start()"))
        {
            // Subscribe to script's reload event to allow live-reload of the application
            SubscribeToEvent(scriptFile_, E_RELOADSTARTED, HANDLER(ClockworkPlayer, HandleScriptReloadStarted));
            SubscribeToEvent(scriptFile_, E_RELOADFINISHED, HANDLER(ClockworkPlayer, HandleScriptReloadFinished));
            SubscribeToEvent(scriptFile_, E_RELOADFAILED, HANDLER(ClockworkPlayer, HandleScriptReloadFailed));
            return;
        }
#else
        ErrorExit("AngelScript is not enabled!");
        return;
#endif
    }
    else
    {
#ifdef CLOCKWORK_LUA
        // Instantiate and register the Lua script subsystem
        LuaScript* luaScript = new LuaScript(context_);
        context_->RegisterSubsystem(luaScript);

        // If script loading is successful, proceed to main loop
        if (luaScript->ExecuteFile(scriptFileName_))
        {
            luaScript->ExecuteFunction("Start");
            return;
        }
#else
        ErrorExit("Lua is not enabled!");
        return;
#endif
    }

    // The script was not successfully loaded. Show the last error message and do not run the main loop
    ErrorExit();
}

void ClockworkPlayer::Stop()
{
#ifdef CLOCKWORK_ANGELSCRIPT
    if (scriptFile_)
    {
        // Execute the optional stop function
        if (scriptFile_->GetFunction("void Stop()"))
            scriptFile_->Execute("void Stop()");
    }
#else
    if (false)
    {
    }
#endif

#ifdef CLOCKWORK_LUA
    else
    {
        LuaScript* luaScript = GetSubsystem<LuaScript>();
        if (luaScript && luaScript->GetFunction("Stop", true))
            luaScript->ExecuteFunction("Stop");
    }
#endif
}

void ClockworkPlayer::HandleScriptReloadStarted(StringHash eventType, VariantMap& eventData)
{
#ifdef CLOCKWORK_ANGELSCRIPT
    if (scriptFile_->GetFunction("void Stop()"))
        scriptFile_->Execute("void Stop()");
#endif
}

void ClockworkPlayer::HandleScriptReloadFinished(StringHash eventType, VariantMap& eventData)
{
#ifdef CLOCKWORK_ANGELSCRIPT
    // Restart the script application after reload
    if (!scriptFile_->Execute("void Start()"))
    {
        scriptFile_.Reset();
        ErrorExit();
    }
#endif
}

void ClockworkPlayer::HandleScriptReloadFailed(StringHash eventType, VariantMap& eventData)
{
#ifdef CLOCKWORK_ANGELSCRIPT
    scriptFile_.Reset();
    ErrorExit();
#endif
}