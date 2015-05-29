#include <Clockwork/Clockwork.h>

#include <Clockwork/Graphics/Camera.h>
#include <Clockwork/Core/CoreEvents.h>
#include <Clockwork/Engine/Engine.h>
#include <Clockwork/UI/Font.h>
#include <Clockwork/Graphics/Graphics.h>
#include <Clockwork/Input/Input.h>
#include <Clockwork/LuaScript/LuaFile.h>
#include <Clockwork/LuaScript/LuaFunction.h>
#include <Clockwork/LuaScript/LuaScript.h>
#include <Clockwork/LuaScript/LuaScriptInstance.h>
#include <Clockwork/Graphics/Material.h>
#include <Clockwork/Graphics/Model.h>
#include <Clockwork/Graphics/Octree.h>
#include <Clockwork/Graphics/Renderer.h>
#include <Clockwork/Resource/ResourceCache.h>
#include <Clockwork/Scene/Scene.h>
#include <Clockwork/Graphics/StaticModel.h>
#include <Clockwork/UI/Text.h>
#include <Clockwork/UI/UI.h>
#include <Clockwork/Graphics/Zone.h>

#include "LuaIntegration.h"

#include <Clockwork/DebugNew.h>

DEFINE_APPLICATION_MAIN(LuaIntegration)

LuaIntegration::LuaIntegration(Context* context) :
    Sample(context)
{
    // Instantiate and register the Lua script subsystem so that we can use the LuaScriptInstance component
    context_->RegisterSubsystem(new LuaScript(context_));
}

void LuaIntegration::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void LuaIntegration::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create the Octree component to the scene so that drawable objects can be rendered. Use default volume
    // (-1000, -1000, -1000) to (1000, 1000, 1000)
    scene_->CreateComponent<Octree>();

    // Create a Zone component into a child scene node. The Zone controls ambient lighting and fog settings. Like the Octree,
    // it also defines its volume with a bounding box, but can be rotated (so it does not need to be aligned to the world X, Y
    // and Z axes.) Drawable objects "pick up" the zone they belong to and use it when rendering; several zones can exist
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    // Set same volume as the Octree, set a close bluish fog and some ambient light
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.05f, 0.1f, 0.15f));
    zone->SetFogColor(Color(0.1f, 0.2f, 0.3f));
    zone->SetFogStart(10.0f);
    zone->SetFogEnd(100.0f);

    LuaFile* scriptFile = cache->GetResource<LuaFile>("LuaScripts/Rotator.lua");
    if (!scriptFile)
        return;

    // Create randomly positioned and oriented box StaticModels in the scene
    const unsigned NUM_OBJECTS = 2000;
    for (unsigned i = 0; i < NUM_OBJECTS; ++i)
    {
        Node* boxNode = scene_->CreateChild("Box");
        boxNode->SetPosition(Vector3(Random(200.0f) - 100.0f, Random(200.0f) - 100.0f, Random(200.0f) - 100.0f));
        // Orient using random pitch, yaw and roll Euler angles
        boxNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
        StaticModel* boxObject = boxNode->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

        // Add our custom Rotator script object (using the LuaScriptInstance C++ component to instantiate / store it) which will
        // rotate the scene node each frame, when the scene sends its update event
        LuaScriptInstance* instance = boxNode->CreateComponent<LuaScriptInstance>();
        instance->CreateObject(scriptFile, "Rotator");

        // Call the script object's "SetRotationSpeed" function.
        LuaFunction* function = instance->GetScriptObjectFunction("SetRotationSpeed");
        if (function && function->BeginCall(instance))
        {
            function->PushUserType(Vector3(10.0f, 20.0f, 30.0f), "Vector3");
            function->EndCall();
        }
    }

    // Create the camera. Let the starting position be at the world origin. As the fog limits maximum visible distance, we can
    // bring the far clip plane closer for more effective culling of distant objects
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(100.0f);

    // Create a point light to the camera scene node
    Light* light = cameraNode_->CreateComponent<Light>();
    light->SetLightType(LIGHT_POINT);
    light->SetRange(30.0f);
}

void LuaIntegration::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use WASD keys and mouse/touch to move");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

void LuaIntegration::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void LuaIntegration::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, HANDLER(LuaIntegration, HandleUpdate));
}

void LuaIntegration::MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
}

void LuaIntegration::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);
}