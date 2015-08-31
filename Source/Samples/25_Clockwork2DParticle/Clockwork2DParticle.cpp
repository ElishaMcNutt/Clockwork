//
// Copyright (c) 2008-2015 the Clockwork project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Clockwork/Clockwork.h>

#include <Clockwork/Graphics/Camera.h>
#include <Clockwork/Core/CoreEvents.h>
#include <Clockwork/Engine/Engine.h>
#include <Clockwork/UI/Font.h>
#include <Clockwork/Graphics/Graphics.h>
#include <Clockwork/Input/Input.h>
#include <Clockwork/Input/InputEvents.h>
#include <Clockwork/Graphics/Octree.h>
#include <Clockwork/Clockwork2D/ParticleEmitter2D.h>
#include <Clockwork/Clockwork2D/ParticleEffect2D.h>
#include <Clockwork/Graphics/Renderer.h>
#include <Clockwork/Resource/ResourceCache.h>
#include <Clockwork/Scene/Scene.h>
#include <Clockwork/UI/Text.h>
#include <Clockwork/Graphics/Zone.h>

#include "Clockwork2DParticle.h"

#include <Clockwork/DebugNew.h>

DEFINE_APPLICATION_MAIN(Clockwork2DParticle)

Clockwork2DParticle::Clockwork2DParticle(Context* context) :
    Sample(context)
{
}

void Clockwork2DParticle::Start()
{
    // Execute base class startup
    Sample::Start();

    // Set mouse visibile
    Input* input = GetSubsystem<Input>();
    input->SetMouseVisible(true);

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void Clockwork2DParticle::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();

    // Create camera node
    cameraNode_ = scene_->CreateChild("Camera");
    // Set camera's position
    cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -10.0f));

    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetOrthographic(true);

    Graphics* graphics = GetSubsystem<Graphics>();
    camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);
    camera->SetZoom(1.2f * Min((float)graphics->GetWidth() / 1280.0f, (float)graphics->GetHeight() / 800.0f)); // Set zoom according to user's resolution to ensure full visibility (initial zoom (1.2) is set for full visibility at 1280x800 resolution)

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    ParticleEffect2D* particleEffect = cache->GetResource<ParticleEffect2D>("Clockwork2D/sun.pex");
    if (!particleEffect)
        return;

    particleNode_ = scene_->CreateChild("ParticleEmitter2D");
    ParticleEmitter2D* particleEmitter = particleNode_->CreateComponent<ParticleEmitter2D>();
    particleEmitter->SetEffect(particleEffect);

    ParticleEffect2D* greenSpiralEffect = cache->GetResource<ParticleEffect2D>("Clockwork2D/greenspiral.pex");
    if (!greenSpiralEffect)
        return;

    Node* greenSpiralNode = scene_->CreateChild("GreenSpiral");
    ParticleEmitter2D* greenSpiralEmitter = greenSpiralNode->CreateComponent<ParticleEmitter2D>();
    greenSpiralEmitter->SetEffect(greenSpiralEffect);
}

void Clockwork2DParticle::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use mouse/touch to move the particle.");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
}

void Clockwork2DParticle::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void Clockwork2DParticle::SubscribeToEvents()
{
    SubscribeToEvent(E_MOUSEMOVE, HANDLER(Clockwork2DParticle, HandleMouseMove));
    if (touchEnabled_)
        SubscribeToEvent(E_TOUCHMOVE, HANDLER(Clockwork2DParticle, HandleMouseMove));

    // Unsubscribe the SceneUpdate event from base class to prevent camera pitch and yaw in 2D sample
    UnsubscribeFromEvent(E_SCENEUPDATE);
}

void Clockwork2DParticle::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    if (particleNode_)
    {
        using namespace MouseMove;
        float x = (float)eventData[P_X].GetInt();
        float y = (float)eventData[P_Y].GetInt();
        Graphics* graphics = GetSubsystem<Graphics>();
        Camera* camera = cameraNode_->GetComponent<Camera>();
        particleNode_->SetPosition(camera->ScreenToWorldPoint(Vector3(x / graphics->GetWidth(), y / graphics->GetHeight(), 10.0f)));
    }
}
