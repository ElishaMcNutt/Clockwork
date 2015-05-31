-- CrowdNavigation example.
-- This sample demonstrates:
--     - Generating a dynamic navigation mesh into the scene
--     - Performing path queries to the navigation mesh
--     - Adding and removing obstacles/agents at runtime
--     - Raycasting drawable components
--     - Crowd movement management
--     - Accessing crowd agents with the crowd manager
--     - Using off-mesh connections to make boxes climbable
--     - Using agents to simulate moving obstacles

require "LuaScripts/Utilities/Sample"

local navMesh = nil
local crowdManager = nil
local agents = {}
local NUM_BARRELS = 20

function Start()
    -- Execute the common startup for samples
    SampleStart()

    -- Create the scene content
    CreateScene()

    -- Create the UI content
    CreateUI()

    -- Setup the viewport for displaying the scene
    SetupViewport()

    -- Hook up to the frame update and render post-update events
    SubscribeToEvents()
end

function CreateScene()
    scene_ = Scene()
    -- Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    -- Also create a DebugRenderer component so that we can draw debug geometry
    scene_:CreateComponent("Octree")
    scene_:CreateComponent("DebugRenderer")

    -- Create scene node & StaticModel component for showing a static plane
    local planeNode = scene_:CreateChild("Plane")
    planeNode.scale = Vector3(100.0, 1.0, 100.0)
    local planeObject = planeNode:CreateComponent("StaticModel")
    planeObject.model = cache:GetResource("Model", "Models/Plane.mdl")
    planeObject.material = cache:GetResource("Material", "Materials/StoneTiled.xml")

    -- Create a Zone component for ambient lighting & fog control
    local zoneNode = scene_:CreateChild("Zone")
    local zone = zoneNode:CreateComponent("Zone")
    zone.boundingBox = BoundingBox(-1000.0, 1000.0)
    zone.ambientColor = Color(0.15, 0.15, 0.15)
    zone.fogColor = Color(0.5, 0.5, 0.7)
    zone.fogStart = 100.0
    zone.fogEnd = 300.0

    -- Create a directional light to the world. Enable cascaded shadows on it
    local lightNode = scene_:CreateChild("DirectionalLight")
    lightNode.direction = Vector3(0.6, -1.0, 0.8)
    local light = lightNode:CreateComponent("Light")
    light.lightType = LIGHT_DIRECTIONAL
    light.castShadows = true
    light.shadowBias = BiasParameters(0.00025, 0.5)
    -- Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light.shadowCascade = CascadeParameters(10.0, 50.0, 200.0, 0.0, 0.8)

    -- Create randomly sized boxes. If boxes are big enough, make them occluders. Occluders will be software rasterized before
    -- rendering to a low-resolution depth-only buffer to test the objects in the view frustum for visibility
    local boxes = {}
    for i = 1, 20 do
        local boxNode = scene_:CreateChild("Box")
        local size = 1.0 + Random(10.0)
        boxNode.position = Vector3(Random(80.0) - 40.0, size * 0.5, Random(80.0) - 40.0)
        boxNode:SetScale(size)
        local boxObject = boxNode:CreateComponent("StaticModel")
        boxObject.model = cache:GetResource("Model", "Models/Box.mdl")
        boxObject.material = cache:GetResource("Material", "Materials/Stone.xml")
        boxObject.castShadows = true
        if size >= 3.0 then
            boxObject.occluder = true
            table.insert(boxes, boxNode)
        end
    end

    -- Create a DynamicNavigationMesh component to the scene root
    navMesh = scene_:CreateComponent("DynamicNavigationMesh")
    -- Enable drawing debug geometry for obstacles and off-mesh connections
    navMesh.drawObstacles = true
    navMesh.drawOffMeshConnections = true
    -- Set the agent height large enough to exclude the layers under boxes
    navMesh.agentHeight = 10
    -- Set nav mesh cell height to minimum (allows agents to be grounded)
    navMesh.cellHeight = 0.05
    -- Create a Navigable component to the scene root. This tags all of the geometry in the scene as being part of the
    -- navigation mesh. By default this is recursive, but the recursion could be turned off from Navigable
    scene_:CreateComponent("Navigable")
    -- Add padding to the navigation mesh in Y-direction so that we can add objects on top of the tallest boxes
    -- in the scene and still update the mesh correctly
    navMesh.padding = Vector3(0.0, 10.0, 0.0)
    -- Now build the navigation geometry. This will take some time. Note that the navigation mesh will prefer to use
    -- physics geometry from the scene nodes, as it often is simpler, but if it can not find any (like in this example)
    -- it will use renderable geometry instead
    navMesh:Build()

    -- Create an off-mesh connection for each box to make it climbable (tiny boxes are skipped).
    -- Note that OffMeshConnections must be added before building the navMesh, but as we are adding Obstacles next, tiles will be automatically rebuilt.
    -- Creating connections post-build here allows us to use FindNearestPoint() to procedurally set accurate positions for the connection
    CreateBoxOffMeshConnections(boxes)

    -- Create some mushrooms as obstacles. Note that obstacles are non-walkable areas
    for i = 1, 100 do
        CreateMushroom(Vector3(Random(90.0) - 45.0, 0.0, Random(90.0) - 45.0))
    end

    -- Create a DetourCrowdManager component to the scene root (mandatory for crowd agents)
    crowdManager = scene_:CreateComponent("DetourCrowdManager")

    -- Create some movable barrels. We create them as crowd agents, as for moving entities it is less expensive and more convenient than using obstacles
    CreateMovingBarrels()

    -- Create Jack node as crowd agent
    SpawnJack(Vector3(-5, 0, 20))

    -- Create the camera. Limit far clip distance to match the fog. Note: now we actually create the camera node outside
    -- the scene, because we want it to be unaffected by scene load / save
    cameraNode = Node()
    local camera = cameraNode:CreateComponent("Camera")
    camera.farClip = 300.0

    -- Set an initial position for the camera scene node above the plane
    cameraNode.position = Vector3(0.0, 5.0, 0.0)
end

function CreateUI()
    -- Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will
    -- control the camera, and when visible, it will point the raycast target
    local style = cache:GetResource("XMLFile", "UI/DefaultStyle.xml")
    local cursor = Cursor:new()
    cursor:SetStyleAuto(style)
    ui.cursor = cursor
    -- Set starting position of the cursor at the rendering window center
    cursor:SetPosition(graphics.width / 2, graphics.height / 2)

    -- Construct new Text object, set string to display and font to use
    local instructionText = ui.root:CreateChild("Text")
    instructionText.text = "Use WASD keys to move, RMB to rotate view\n"..
        "LMB to set destination, SHIFT+LMB to spawn a Jack\n"..
        "CTRL+LMB to teleport main agent\n"..
        "MMB to add obstacles or remove obstacles/agents\n"..
        "F5 to save scene, F7 to load\n"..
        "Space to toggle debug geometry"
    instructionText:SetFont(cache:GetResource("Font", "Fonts/Anonymous Pro.ttf"), 12)
    -- The text has multiple rows. Center them in relation to each other
    instructionText.textAlignment = HA_CENTER

    -- Position the text relative to the screen center
    instructionText.horizontalAlignment = HA_CENTER
    instructionText.verticalAlignment = VA_CENTER
    instructionText:SetPosition(0, ui.root.height / 4)
end

function SetupViewport()
    -- Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    local viewport = Viewport:new(scene_, cameraNode:GetComponent("Camera"))
    renderer:SetViewport(0, viewport)
end

function SubscribeToEvents()
    -- Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent("Update", "HandleUpdate")

    -- Subscribe HandlePostRenderUpdate() function for processing the post-render update event, during which we request
    -- debug geometry
    SubscribeToEvent("PostRenderUpdate", "HandlePostRenderUpdate")

    -- Subscribe HandleCrowdAgentFailure() function for resolving invalidation issues with agents, during which we
    -- use a larger extents for finding a point on the navmesh to fix the agent's position
    SubscribeToEvent("CrowdAgentFailure", "HandleCrowdAgentFailure")
end

function SpawnJack(pos)
    local jackNode = scene_:CreateChild("Jack")
    jackNode.position = pos
    local modelObject = jackNode:CreateComponent("AnimatedModel")
    modelObject.model = cache:GetResource("Model", "Models/Jack.mdl")
    modelObject.material = cache:GetResource("Material", "Materials/Jack.xml")
    modelObject.castShadows = true
    jackNode:CreateComponent("AnimationController")

    -- Create a CrowdAgent component and set its height and realistic max speed/acceleration. Use default radius
    local agent = jackNode:CreateComponent("CrowdAgent")
    agent.height = 2.0
    agent.maxSpeed = 4.0
    agent.maxAccel = 100.0
    agents = crowdManager:GetActiveAgents() -- Update agents container
end

function CreateMushroom(pos)
    local mushroomNode = scene_:CreateChild("Mushroom")
    mushroomNode.position = pos
    mushroomNode.rotation = Quaternion(0.0, Random(360.0), 0.0)
    mushroomNode:SetScale(2.0 + Random(0.5))
    local mushroomObject = mushroomNode:CreateComponent("StaticModel")
    mushroomObject.model = cache:GetResource("Model", "Models/Mushroom.mdl")
    mushroomObject.material = cache:GetResource("Material", "Materials/Mushroom.xml")
    mushroomObject.castShadows = true

    -- Create the navigation Obstacle component and set its height & radius proportional to scale
    local obstacle = mushroomNode:CreateComponent("Obstacle")
    obstacle.radius = mushroomNode.scale.x
    obstacle.height = mushroomNode.scale.y
end

function CreateBoxOffMeshConnections(boxes)
    for i, box in ipairs(boxes) do
        local boxPos = box.position
        local boxHalfSize = box.scale.x / 2

        -- Create 2 empty nodes for the start & end points of the connection. Note that order matters only when using one-way/unidirectional connection.
        local connectionStart = box:CreateChild("ConnectionStart")
        connectionStart.worldPosition = navMesh:FindNearestPoint(boxPos + Vector3(boxHalfSize, -boxHalfSize, 0)) -- Base of box
        local connectionEnd = connectionStart:CreateChild("ConnectionEnd")
        connectionEnd.worldPosition = navMesh:FindNearestPoint(boxPos + Vector3(boxHalfSize, boxHalfSize, 0)) -- Top of box

        -- Create the OffMeshConnection component to one node and link the other node
        local connection = connectionStart:CreateComponent("OffMeshConnection")
        connection.endPoint = connectionEnd
    end
end

function CreateMovingBarrels()
    local barrel = scene_:CreateChild("Barrel")
    local model = barrel:CreateComponent("StaticModel")
    model.model = cache:GetResource("Model", "Models/Cylinder.mdl")
    model.material = cache:GetResource("Material", "Materials/StoneTiled.xml")
    model.material:SetTexture(TU_DIFFUSE, cache:GetResource("Texture2D", "Textures/TerrainDetail2.dds"))
    model.castShadows = true
    for i = 1, NUM_BARRELS do
        local clone = barrel:Clone()
        local size = 0.5 + Random(1)
        clone.scale = Vector3(size / 1.5, size * 2, size / 1.5)
        clone.position = navMesh:FindNearestPoint(Vector3(Random(80.0) - 40.0, size * 0.5 , Random(80.0) - 40.0))
        local agent = clone:CreateComponent("CrowdAgent")
        agent.radius = clone.scale.x * 0.5
        agent.height = size
    end
    barrel:Remove()
end

function SetPathPoint()
    local hitPos, hitDrawable = Raycast(250.0)

    if hitDrawable then
        local pathPos = navMesh:FindNearestPoint(hitPos, Vector3.ONE)

        if input:GetQualifierDown(QUAL_SHIFT) then
            -- Spawn a Jack
            SpawnJack(pathPos)

        elseif input:GetQualifierDown(QUAL_CTRL) and table.maxn(agents) > NUM_BARRELS then
            -- Teleport
            local agent = agents[NUM_BARRELS + 1] -- Get first Jack agent
            local node = agent.node
            node:LookAt(pathPos) -- Face target
            agent:SetMoveVelocity(Vector3.ZERO) -- Stop agent
            node.position = pathPos

        else
            -- Set target position and init agents' move
            for i = NUM_BARRELS + 1, table.maxn(agents) do
                local agent = agents[i]
                if i == NUM_BARRELS + 1 then
                    -- The first Jack agent will always move to the exact position and is strong enough to push barrels and his siblings (no avoidance)
                    agent.navigationPushiness = PUSHINESS_HIGH
                    agent:SetMoveTarget(pathPos)
                else
                    -- Other Jack agents will move to a random point nearby
                    local targetPos = navMesh:FindNearestPoint(pathPos + Vector3(Random(-4.5, 4.5), 0, Random(-4.5, 4.5)), Vector3.ONE)
                    agent:SetMoveTarget(targetPos)
                end
            end
        end
    end
end

function AddOrRemoveObject()
    -- Raycast and check if we hit a mushroom node. If yes, remove it, if no, create a new one
    local hitPos, hitDrawable = Raycast(250.0)
    if hitDrawable then

        local hitNode = hitDrawable.node
        if hitNode.name == "Mushroom" then
            hitNode:Remove()
        elseif hitNode.name == "Jack" then
            hitNode:Remove()
            agents = crowdManager:GetActiveAgents() -- Update agents container
        else
            CreateMushroom(hitPos)
        end
    end
end

function Raycast(maxDistance)
    local pos = ui.cursorPosition
    -- Check the cursor is visible and there is no UI element in front of the cursor
    if (not ui.cursor.visible) or (ui:GetElementAt(pos, true) ~= nil) then
        return nil, nil
    end

    local camera = cameraNode:GetComponent("Camera")
    local cameraRay = camera:GetScreenRay(pos.x / graphics.width, pos.y / graphics.height)
    -- Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    local octree = scene_:GetComponent("Octree")
    local result = octree:RaycastSingle(cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY)
    if result.drawable ~= nil then
        return result.position, result.drawable
    end

    return nil, nil
end

function MoveCamera(timeStep)
    -- Right mouse button controls mouse cursor visibility: hide when pressed
    ui.cursor.visible = not input:GetMouseButtonDown(MOUSEB_RIGHT)

    -- Do not move if the UI has a focused element (the console)
    if ui.focusElement ~= nil then
        return
    end

    -- Movement speed as world units per second
    local MOVE_SPEED = 20.0
    -- Mouse sensitivity as degrees per pixel
    local MOUSE_SENSITIVITY = 0.1

    -- Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    -- Only move the camera when the cursor is hidden
    if not ui.cursor.visible then
        local mouseMove = input.mouseMove
        yaw = yaw + MOUSE_SENSITIVITY * mouseMove.x
        pitch = pitch + MOUSE_SENSITIVITY * mouseMove.y
        pitch = Clamp(pitch, -90.0, 90.0)

        -- Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
        cameraNode.rotation = Quaternion(pitch, yaw, 0.0)
    end

    -- Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if input:GetKeyDown(KEY_W) then
        cameraNode:Translate(Vector3(0.0, 0.0, 1.0) * MOVE_SPEED * timeStep)
    end
    if input:GetKeyDown(KEY_S) then
        cameraNode:Translate(Vector3(0.0, 0.0, -1.0) * MOVE_SPEED * timeStep)
    end
    if input:GetKeyDown(KEY_A) then
        cameraNode:Translate(Vector3(-1.0, 0.0, 0.0) * MOVE_SPEED * timeStep)
    end
    if input:GetKeyDown(KEY_D) then
        cameraNode:Translate(Vector3(1.0, 0.0, 0.0) * MOVE_SPEED * timeStep)
    end
    -- Set destination or spawn a jack with left mouse button
    if input:GetMouseButtonPress(MOUSEB_LEFT) then
        SetPathPoint()
    end
    -- Add new obstacle or remove existing obstacle/agent with middle mouse button
    if input:GetMouseButtonPress(MOUSEB_MIDDLE) then
        AddOrRemoveObject()
    end

    -- Check for loading/saving the scene from/to the file Data/Scenes/CrowdNavigation.xml relative to the executable directory
    if input:GetKeyPress(KEY_F5) then
        scene_:SaveXML(fileSystem:GetProgramDir().."Data/Scenes/CrowdNavigation.xml")
    end
    if input:GetKeyPress(KEY_F7) then
        scene_:LoadXML(fileSystem:GetProgramDir().."Data/Scenes/CrowdNavigation.xml")
        -- After reload, reacquire navMesh, crowd manager & agents
        navMesh = scene_:GetComponent("DynamicNavigationMesh")
        crowdManager = scene_:GetComponent("DetourCrowdManager")
        agents = crowdManager:GetActiveAgents()
    end

    -- Toggle debug geometry with space
    if input:GetKeyPress(KEY_SPACE) then
        drawDebug = not drawDebug
    end
end

function HandleUpdate(eventType, eventData)
    -- Take the frame time step, which is stored as a float
    local timeStep = eventData:GetFloat("TimeStep")

    -- Move the camera, scale movement with time step
    MoveCamera(timeStep)

    -- Make the Jack CrowdAgents face the direction of their velocity and update animation
    for i = NUM_BARRELS + 1, table.maxn(agents) do
        local agent = agents[i]
        local node = agent.node
        local animCtrl = node:GetComponent("AnimationController")
        local velocity = agent.actualVelocity

        if velocity:Length() < 0.6 then
            animCtrl:Stop("Models/Jack_Walk.ani", 0.2)
        else
            node.worldDirection = velocity
            animCtrl:PlayExclusive("Models/Jack_Walk.ani", 0, true, 0.2)
            animCtrl:SetSpeed("Models/Jack_Walk.ani", velocity:Length() * 0.3)
        end
    end
end

function HandlePostRenderUpdate(eventType, eventData)
    if drawDebug then
        -- Visualize navigation mesh, obstacles and off-mesh connections
        navMesh:DrawDebugGeometry(true)
        -- Visualize agents' path and position to reach
        crowdManager:DrawDebugGeometry(true)
    end
end

function HandleCrowdAgentFailure(eventType, eventData)
    local node = eventData:GetPtr("Node", "Node")
    local agentState = eventData:GetInt("CrowdAgentState")

    -- If the agent's state is invalid, likely from spawning on the side of a box, find a point in a larger area
    if agentState == CROWD_AGENT_INVALID then
        -- Get a point on the navmesh using more generous extents
        local newPos = navMesh:FindNearestPoint(node.position, Vector3(5, 5, 5))
        -- Set the new node position, CrowdAgent component will automatically reset the state of the agent
        node.position = newPos
    end
end

-- Create XML patch instructions for screen joystick layout specific to this sample app
function GetScreenJoystickPatchString()
    return
        "<patch>" ..
        "    <add sel=\"/element\">" ..
        "        <element type=\"Button\">" ..
        "            <attribute name=\"Name\" value=\"Button3\" />" ..
        "            <attribute name=\"Position\" value=\"-120 -120\" />" ..
        "            <attribute name=\"Size\" value=\"96 96\" />" ..
        "            <attribute name=\"Horiz Alignment\" value=\"Right\" />" ..
        "            <attribute name=\"Vert Alignment\" value=\"Bottom\" />" ..
        "            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />" ..
        "            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />" ..
        "            <attribute name=\"Hover Image Offset\" value=\"0 0\" />" ..
        "            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />" ..
        "            <element type=\"Text\">" ..
        "                <attribute name=\"Name\" value=\"Label\" />" ..
        "                <attribute name=\"Horiz Alignment\" value=\"Center\" />" ..
        "                <attribute name=\"Vert Alignment\" value=\"Center\" />" ..
        "                <attribute name=\"Color\" value=\"0 0 0 1\" />" ..
        "                <attribute name=\"Text\" value=\"Spawn Jack\" />" ..
        "            </element>" ..
        "            <element type=\"Text\">" ..
        "                <attribute name=\"Name\" value=\"KeyBinding\" />" ..
        "                <attribute name=\"Text\" value=\"LSHIFT\" />" ..
        "            </element>" ..
        "            <element type=\"Text\">" ..
        "                <attribute name=\"Name\" value=\"MouseButtonBinding\" />" ..
        "                <attribute name=\"Text\" value=\"LEFT\" />" ..
        "            </element>" ..
        "        </element>" ..
        "        <element type=\"Button\">" ..
        "            <attribute name=\"Name\" value=\"Button4\" />" ..
        "            <attribute name=\"Position\" value=\"-120 -12\" />" ..
        "            <attribute name=\"Size\" value=\"96 96\" />" ..
        "            <attribute name=\"Horiz Alignment\" value=\"Right\" />" ..
        "            <attribute name=\"Vert Alignment\" value=\"Bottom\" />" ..
        "            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />" ..
        "            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />" ..
        "            <attribute name=\"Hover Image Offset\" value=\"0 0\" />" ..
        "            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />" ..
        "            <element type=\"Text\">" ..
        "                <attribute name=\"Name\" value=\"Label\" />" ..
        "                <attribute name=\"Horiz Alignment\" value=\"Center\" />" ..
        "                <attribute name=\"Vert Alignment\" value=\"Center\" />" ..
        "                <attribute name=\"Color\" value=\"0 0 0 1\" />" ..
        "                <attribute name=\"Text\" value=\"Obstacles\" />" ..
        "            </element>" ..
        "            <element type=\"Text\">" ..
        "                <attribute name=\"Name\" value=\"MouseButtonBinding\" />" ..
        "                <attribute name=\"Text\" value=\"MIDDLE\" />" ..
        "            </element>" ..
        "        </element>" ..
        "    </add>" ..
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />" ..
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Set</replace>" ..
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">" ..
        "        <element type=\"Text\">" ..
        "            <attribute name=\"Name\" value=\"MouseButtonBinding\" />" ..
        "            <attribute name=\"Text\" value=\"LEFT\" />" ..
        "        </element>" ..
        "    </add>" ..
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />" ..
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Debug</replace>" ..
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">" ..
        "        <element type=\"Text\">" ..
        "            <attribute name=\"Name\" value=\"KeyBinding\" />" ..
        "            <attribute name=\"Text\" value=\"SPACE\" />" ..
        "        </element>" ..
        "    </add>" ..
        "</patch>"
end
