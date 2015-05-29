#include "../Core/Context.h"
#include "../IO/Deserializer.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../LuaScript/LuaFile.h"
#include "../Core/ProcessUtils.h"
#include "../IO/Serializer.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include "../DebugNew.h"

namespace Clockwork
{

LuaFile::LuaFile(Context* context) :
    Resource(context),
    size_(0),
    hasLoaded_(false),
    hasExecuted_(false)
{

}

LuaFile::~LuaFile()
{

}

void LuaFile::RegisterObject(Context* context)
{
    context->RegisterFactory<LuaFile>();
}

bool LuaFile::BeginLoad(Deserializer& source)
{
    size_ = source.GetSize();

    if (size_ == 0)
        return false;

    // Read all data.
    data_ = new char[size_];
    if (source.Read(data_, size_) != size_)
        return false;

    SetMemoryUse(size_);

    return true;
}

bool LuaFile::Save(Serializer& dest) const
{
    if (size_ == 0)
        return false;

    dest.Write(data_, size_);

    return true;
}


bool LuaFile::LoadChunk(lua_State* luaState)
{
    if (hasLoaded_)
        return true;

    if (size_ == 0)
        return false;

    if (!luaState)
        return false;

    int top = lua_gettop(luaState);

    // Get file base name
    String name = GetName();
    unsigned extPos = name.FindLast('.');
    if (extPos != String::NPOS)
    {
        name = name.Substring(0, extPos);
    }

    int error = luaL_loadbuffer(luaState, data_, size_, name.CString());
    if (error)
    {
        const char* message = lua_tostring(luaState, -1);
        LOGERROR("Load Buffer failed for " + GetName() + ": " + String(message));
        lua_settop(luaState, top);
        return false;
    }

    LOGINFO("Loaded Lua script " + GetName());
    hasLoaded_ = true;

    return true;
}

bool LuaFile::LoadAndExecute(lua_State* luaState)
{
    if (hasExecuted_)
        return true;

    if (!LoadChunk(luaState))
        return false;

    int top = lua_gettop(luaState);

    if (lua_pcall(luaState, 0, 0, 0))
    {
        const char* message = lua_tostring(luaState, -1);
        LOGERROR("Lua Execute failed for " + GetName() + ": " + String(message));
        lua_settop(luaState, top);
        return false;
    }

    hasExecuted_ = true;

    return true;
}

}