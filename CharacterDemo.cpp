//
// Copyright (c) 2008-2016 the Urho3D project.
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

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Engine/Console.h>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"

#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>

#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include <Urho3D/DebugNew.h>

static const StringHash E_CUSTOMEVENT("CustomEvent");
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
static const StringHash PLAYER_ID("IDENTITY");
static const StringHash E_CLIENTISREADY("ClientReadyToStart");

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

CharacterDemo::CharacterDemo(Context* context) :
	Sample(context),
	firstPerson_(false),
	MAX_MISSLES(10),
	drawDebug_(false)
{
}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Start()
{
	Sample::Start();
	OpenConsoleWindow();
	cache = GetSubsystem<ResourceCache>();
	gs = NONE;
	CreateMainMenu();
	SubscribeToEvents();
}

void CharacterDemo::CreateServerScene()
{
	// SCENE CREATION
	cache = GetSubsystem<ResourceCache>();

	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);
	scene_->CreateComponent<DebugRenderer>(LOCAL);

	// CAMERA CREATION
	cameraNode_ = scene_->CreateChild("Camera", LOCAL);
	//cameraNode_->SetRotation(Quaternion(0.0f, 90.0f, 0.0f));
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 90.0f, 0.0f));
	cameraNode_->SetRotation(Quaternion(90.0f, 0.0f, 0.0f));
	camera->SetFarClip(750.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	// ZONE & FOG CREATION
	Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.09f, 0.301f, 0.647f));
	zone->SetFogStart(1.0f);
	zone->SetFogEnd(90.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	Node* n_skybox = scene_->CreateChild("Skybox", LOCAL);
	Skybox* s_skybox = n_skybox->CreateComponent<Skybox>();
	s_skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	s_skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	bS.Initialise(cache, scene_);

	//CreateCharacter();
	CreateEnvironemnt();
}

void CharacterDemo::CreateClientScene() {
	// SCENE CREATION
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);
	scene_->CreateComponent<DebugRenderer>(LOCAL);

	// CAMERA CREATION
	cameraNode_ = scene_->CreateChild("Camera", LOCAL);
	//cameraNode_->SetRotation(Quaternion(0.0f, 90.0f, 0.0f));
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 00.0f, 0.0f));
	camera->SetFarClip(750.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	// ZONE & FOG CREATION
	Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.09f, 0.301f, 0.647f));
	zone->SetFogStart(1.0f);
	zone->SetFogEnd(90.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	Node* n_skybox = scene_->CreateChild("Skybox", LOCAL);
	Skybox* s_skybox = n_skybox->CreateComponent<Skybox>();
	s_skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	s_skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	if (gs == SINGLEPLAYER) {
		bS.Initialise(cache, scene_);
	}
	CreateEnvironemnt();
}

Node* CharacterDemo::CreateCharacter()
{
	n_sub = scene_->CreateChild("Submarine");
	n_sub->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	n_sub->SetScale(Vector3(0.4f, 0.4f, 0.4f));
	StaticModel* m_sub = n_sub->CreateComponent<StaticModel>();
	m_sub->SetModel(cache->GetResource<Model>("Models/Submarine.mdl"));
	m_sub->ApplyMaterialList();
	RigidBody* rb_sub = n_sub->CreateComponent<RigidBody>();
	rb_sub->SetCollisionLayer(4);
	CollisionShape* cs_sub = n_sub->CreateComponent<CollisionShape>();
	cs_sub->SetBox(Vector3(16.0f, 12.0f, 50.0f));

	// CREATE PLAYER LIGHT
	Node* n_flashlight = n_sub->CreateChild("Flash Light");
	n_flashlight->SetDirection(cameraNode_->GetDirection());
	l_flashlight = n_flashlight->CreateComponent<Light>();
	l_flashlight->SetColor(Color(1.0f, 1.0f, 1.0f));
	l_flashlight->SetLightType(LIGHT_SPOT);
	l_flashlight->SetBrightness(0.5f);
	l_flashlight->SetTemperature(6590.0f);
	l_flashlight->SetRange(100);
	l_flashlight->SetFov(45);
	l_flashlight->SetEnabled(false);

	return n_sub;
}

void CharacterDemo::CreateEnvironemnt()
{
	// CREATE TERRAIN
	Node* n_terrain = scene_->CreateChild("Terrrain");
	n_terrain->SetPosition(Vector3(0.0f, -5.0f, 0.0f));
	t_terrain = n_terrain->CreateComponent<Terrain>();
	t_terrain->SetSpacing(Vector3(0.4f, 0.05f, 0.2f));
	t_terrain->SetSmoothing(true);
	t_terrain->SetHeightMap(cache->GetResource<Image>("Textures/oceanmap.png"));
	t_terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	t_terrain->SetPatchSize(64);
	t_terrain->SetCastShadows(false);
	t_terrain->SetOccluder(true);
	RigidBody* rb_terrain = n_terrain->CreateComponent<RigidBody>();
	rb_terrain->SetCollisionLayer(2);
	CollisionShape* cs_terrain = n_terrain->CreateComponent<CollisionShape>();
	cs_terrain->SetTerrain();

	// CREATE WALL 1
	Node* n_wall_1 = scene_->CreateChild("Wall", LOCAL);
	n_wall_1->SetPosition(Vector3(0.0f, 45.0f, 102.9f));		// distance is terrain size / 2 + wall size / 2
	n_wall_1->SetScale(Vector3(409.6f, 100.0f, 1.0f));		// size is 1024 * terrain spacing 
	StaticModel* m_wall_1 = n_wall_1->CreateComponent<StaticModel>();
	m_wall_1->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	m_wall_1->SetMaterial(cache->GetResource<Material>("Materials/glass.xml"));
	RigidBody* rb_wall_1 = n_wall_1->CreateComponent<RigidBody>();
	rb_wall_1->SetCollisionLayer(2);
	CollisionShape* cs_wall_1 = n_wall_1->CreateComponent<CollisionShape>();
	cs_wall_1->SetBox(Vector3::ONE);

	// CREATE WALL 2
	Node* n_wall_2 = scene_->CreateChild("Wall", LOCAL);
	n_wall_2->SetPosition(Vector3(0.0f, 45.0f, -102.9f));	// distance is terrain size / 2 + wall size / 2
	n_wall_2->SetScale(Vector3(409.6f, 100.0f, 1.0f));		// size is 1024 * spacing 
	StaticModel* m_wall_2 = n_wall_2->CreateComponent<StaticModel>();
	m_wall_2->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	m_wall_2->SetMaterial(cache->GetResource<Material>("Materials/glass.xml"));
	RigidBody* rb_wall_2 = n_wall_2->CreateComponent<RigidBody>();
	rb_wall_2->SetCollisionLayer(2);
	CollisionShape* cs_wall_2 = n_wall_2->CreateComponent<CollisionShape>();
	cs_wall_2->SetBox(Vector3::ONE);

	// CREATE WALL 3
	Node* n_wall_3 = scene_->CreateChild("Wall", LOCAL);
	n_wall_3->SetPosition(Vector3(205.3f, 45.0f, 0.0f));		// distance is terrain size / 2 + wall size / 2
	n_wall_3->SetScale(Vector3(1.0f, 100.0f, 204.8f));		// size is 1024 * spacing 
	StaticModel* m_wall_3 = n_wall_3->CreateComponent<StaticModel>();
	m_wall_3->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	m_wall_3->SetMaterial(cache->GetResource<Material>("Materials/glass.xml"));
	RigidBody* rb_wall_3 = n_wall_3->CreateComponent<RigidBody>();
	rb_wall_3->SetCollisionLayer(2);
	CollisionShape* cs_wall_3 = n_wall_3->CreateComponent<CollisionShape>();
	cs_wall_3->SetBox(Vector3::ONE);

	// CREATE WALL 4
	Node* n_wall_4 = scene_->CreateChild("Wall", LOCAL);
	n_wall_4->SetPosition(Vector3(-205.3f, 45.0f, 0.0f));		// distance is terrain size / 2 + wall size / 2
	n_wall_4->SetScale(Vector3(1.0f, 100.0f, 204.8f));		// size is 1024 * spacing 
	StaticModel* m_wall_4 = n_wall_4->CreateComponent<StaticModel>();
	m_wall_4->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	m_wall_4->SetMaterial(cache->GetResource<Material>("Materials/glass.xml"));
	RigidBody* rb_wall_4 = n_wall_4->CreateComponent<RigidBody>();
	rb_wall_4->SetCollisionLayer(2);
	CollisionShape* cs_wall_4 = n_wall_4->CreateComponent<CollisionShape>();
	cs_wall_4->SetBox(Vector3::ONE);

	// CREATE WALL 5 - CEILING
	Node* n_wall_5 = scene_->CreateChild("Ceiling", LOCAL);
	n_wall_5->SetPosition(Vector3(0.0f, 95.0f, 0.0f));		// distance is terrain size / 2 + wall size / 2
	n_wall_5->SetScale(Vector3(409.6f, 5.0f, 204.8f));		// size is 1024 * spacing 
	StaticModel* m_wall_5 = n_wall_5->CreateComponent<StaticModel>();
	m_wall_5->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	m_wall_5->SetMaterial(cache->GetResource<Material>("Materials/dark grey.xml"));
	RigidBody* rb_wall_5 = n_wall_5->CreateComponent<RigidBody>();
	rb_wall_5->SetCollisionLayer(2);
	CollisionShape* cs_wall_5 = n_wall_5->CreateComponent<CollisionShape>();
	cs_wall_5->SetBox(Vector3::ONE);
}

void CharacterDemo::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

	SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(CharacterDemo, HandleCollision));

	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(CharacterDemo, handleClientConnected));
	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(CharacterDemo, handleClientDisconnected));
	SubscribeToEvent(E_SERVERCONNECTED, URHO3D_HANDLER(CharacterDemo, handleConnectedToServer));
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(CharacterDemo, handlePhysicsPreStep));
	SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(CharacterDemo, handleClientSceneLoaded));

	//SubscribeToEvent(E_CUSTOMEVENT, URHO3D_HANDLER(CharacterDemo, handleCustomEvent));
	//GetSubsystem<Network>()->RegisterRemoteEvent(E_CUSTOMEVENT);

	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(CharacterDemo, handleServerToClientObjectID));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);
}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	UI* ui = GetSubsystem<UI>();
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();
	if (GetSubsystem<UI>()->GetFocusElement()) return;
	Input* input = GetSubsystem<Input>();
	const float MOUSE_SENSITIVITY = 0.1f;
	IntVector2 mouseMove = input->GetMouseMove();
	if (!ui->GetCursor()->IsVisible() && scene_ != nullptr && gs != NONE) {
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);

		if (gs == SINGLEPLAYER ) {
			n_sub->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
			if (input->GetKeyDown(KEY_SHIFT)) MOVE_SPEED *= 10.0f;
			if (input->GetKeyDown(KEY_W)) n_sub->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
			if (input->GetKeyDown(KEY_S)) n_sub->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
			if (input->GetKeyDown(KEY_A)) n_sub->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
			if (input->GetKeyDown(KEY_D)) n_sub->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
			if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
				spawnMissle();
			}
			cameraNode_->SetPosition(n_sub->GetPosition());
			cameraNode_->SetRotation(n_sub->GetRotation());

		
			//TODO somehow get rigidbodies to do this???
			Vector3 pos = n_sub->GetPosition();
			if ((pos.y_ - 2.0f) < t_terrain->GetHeight(pos)) {
				n_sub->SetPosition(Vector3(pos.x_, t_terrain->GetHeight(pos) + 2.0f, pos.z_));
			}
			if ((pos.x_ - 2.0f) > 204.8f) {
				n_sub->SetPosition(Vector3(pos.x_ - 2.0f, pos.y_, pos.z_));
			}
			if ((pos.x_ - 2.0f) < -204.8f) {
				n_sub->SetPosition(Vector3(pos.x_ + 2.0f, pos.y_, pos.z_));
			}
			if ((pos.z_ - 2.0f) > 102.4f) {
				n_sub->SetPosition(Vector3(pos.x_, pos.y_, pos.z_ - 2.0f));
			}
			if ((pos.z_ - 2.0f) < -102.4f) {
				n_sub->SetPosition(Vector3(pos.x_, pos.y_, pos.z_ + 2.0f));
			}
			if ((pos.y_ + 2.0f) > 90.0f) {
				n_sub->SetPosition(Vector3(pos.x_, pos.y_ - 2.0f, pos.z_));
			}
		}

		if (input->GetKeyPress(KEY_P)) {
			drawDebug_ = !drawDebug_;
		}
		if (input->GetKeyPress(KEY_L) && gs != SERVER) {
			l_flashlight->SetEnabled(!l_flashlight->IsEnabled());
		}
		if (input->GetKeyPress(KEY_N)) {
			File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/map.xml", FILE_WRITE);
			scene_->SaveXML(saveFile);
		}
		if (input->GetKeyPress(KEY_B)) {
			File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/map.xml", FILE_READ);
			scene_->LoadXML(loadFile);
		}
		if (gs != CLIENT) {
			bS.Update(timeStep);
		}

		if (clientObjectID_) {
			Node* playerNode = this->scene_->GetNode(clientObjectID_);
			if (playerNode) {
				cameraNode_->SetPosition(playerNode->GetPosition());
			} else {
				std::cout << "no player" << std::endl;
			}
		}
		
	}
	if (input->GetKeyPress(KEY_M)) {
		menuVisible = !menuVisible;
		ui->GetCursor()->SetVisible(menuVisible);
		window_->SetVisible(menuVisible);
	}
}

void CharacterDemo::HandleCollision(StringHash eventType, VariantMap& eventData) {
	using namespace NodeCollision;
	auto* collided = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
	auto* collided2 = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());

	if (collided2->GetNode()->GetName() == "Missile") {
		collided2->GetNode()->Remove();
		missileCount--;
		if (collided->GetNode()->GetName().Contains("Boid_", false)) {
			//bS.boidList.erase(bS.boidList.begin() + collided->GetNode()->GetVar("boid_number").GetInt());
			collided->GetNode()->SetEnabled(false);
			//std::cout << "erased " << collided->GetNode()->GetVar("boid_number").GetInt() << std::endl;
		}
	}
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData) {
	if (gs != NONE) {
		DebugRenderer* dRenderer = scene_->GetComponent<DebugRenderer>();
		if (drawDebug_) {
			PhysicsWorld* pW_ = scene_->GetComponent<PhysicsWorld>();
			pW_->DrawDebugGeometry(dRenderer, true);
		}
	}
}

void CharacterDemo::spawnMissle() {
	if (missileCount < MAX_MISSLES) {
		missileCount++;
		missileLeftOrRight++;
		missle t = missle();
		t.Initialise(cache, scene_);
		
		t.pRigidBody->SetRotation(n_sub->GetRotation());
		if (missileLeftOrRight == 1) {
			t.pRigidBody->SetPosition((n_sub->GetPosition() + (n_sub->GetRotation() * Vector3(-4.0f, -0.6f, 9.0f))));
		} else {
			t.pRigidBody->SetPosition((n_sub->GetPosition() + (n_sub->GetRotation() * Vector3(4.0f, -0.6f, 9.0f))));
			missileLeftOrRight = 0;
		}
		t.pRigidBody->SetLinearVelocity(n_sub->GetDirection() * 100.0f);
	}
}

void CharacterDemo::CreateMainMenu() {

	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(uiStyle);

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);

	window_ = new Window(context_);
	root->AddChild(window_);

	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
	window_->SetMinWidth(200);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("Window");
	window_->SetStyleAuto();

	Text* menuText = menu_->CreateText("Main Menu", 16, window_, font, 16);
	Button* btnSingleplayer = menu_->CreateButton("Singleplayer", 24, window_, font);
	
	Text* serverText = menu_->CreateText("Server", 16, window_, font, 16);
	leIPAddress = menu_->CreateLineEdit("IP Address", 24, window_, font);
	Button* btnConnect = menu_->CreateButton("Connect", 24, window_, font);
	Button* btnCreateServer = menu_->CreateButton("Start Server", 24, window_, font);
	Text* spacer1 = menu_->CreateText("", 16, window_, font, 16);
	Button* btnDisconnect = menu_->CreateButton("Disconnect", 24, window_, font);
	Button* btnQuit = menu_->CreateButton("Quit", 24, window_, font);
	//ToolTip* tp = menu_->CreateToolTip("Exits the application", font, 12, window_);
	//QuitButton->AddChild(tp);

	SubscribeToEvent(btnQuit, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleQuit));
	SubscribeToEvent(btnSingleplayer, E_RELEASED, URHO3D_HANDLER(CharacterDemo, StartSingleplayer));
	SubscribeToEvent(btnConnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleConnect));
	SubscribeToEvent(btnCreateServer, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleCreateServer));
	SubscribeToEvent(btnDisconnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleDisconnect));
	window_->SetVisible(menuVisible);
	//ui->GetCursor()->SetVisible(menuVisible);
	std::cout << "End of createMenu" << std::endl;
}

void CharacterDemo::HandleQuit(StringHash eventType, VariantMap& eventData) {
	engine_->Exit();
}

void CharacterDemo::StartSingleplayer(StringHash eventType, VariantMap& eventData) {
	UI* ui = GetSubsystem<UI>();
	gs = SINGLEPLAYER;
	CreateClientScene();
	window_->SetVisible(false);
	ui->GetCursor()->SetVisible(false);
}

void CharacterDemo::handleConnect(StringHash eventType, VariantMap& eventData) {
	gs = CLIENT;
	CreateClientScene();
	Network* network = GetSubsystem<Network>();
	String address = leIPAddress->GetText().Trimmed();
	if (address.Empty()) address = "localhost";
	network->Connect(address, SERVER_PORT, scene_);

}

void CharacterDemo::handleCreateServer(StringHash eventType, VariantMap& eventData) {
	UI* ui = GetSubsystem<UI>();
	Log::WriteRaw("(handleCreateServer) CALLED");
	gs = SERVER;
	CreateServerScene();
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);
	window_->SetVisible(false);
	ui->GetCursor()->SetVisible(false);
}

void CharacterDemo::handleDisconnect(StringHash eventType, VariantMap& eventData) {

}

void CharacterDemo::handleClientConnected(StringHash eventType, VariantMap& eventData) { 
	Log::WriteRaw("(handleClientConnected) CALLED");
	using namespace ClientConnected;

	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);
}

void CharacterDemo::handleClientDisconnected(StringHash eventType, VariantMap& eventData) { 
	using namespace ClientConnected;
}

void CharacterDemo::handlePhysicsPreStep(StringHash eventType, VariantMap& eventData) { 
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();
	if (serverConnection) {
		serverConnection->SetPosition(cameraNode_->GetPosition());
		serverConnection->SetControls(FromClientToServerControls());
	} else if (network->IsServerRunning()) {
		ProcessClientControls();
	}
}

Controls CharacterDemo::FromClientToServerControls() {
	Input* input = GetSubsystem<Input>();
	Controls controls;
	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.yaw_ = yaw_;
	return controls;
}

void CharacterDemo::ProcessClientControls() {
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection>>& connections = network->GetClientConnections();
	for (unsigned i = 0; i < connections.Size(); ++i) {
		Connection* connection = connections[i];
		
		Node* playerNode = serverObjects_[connection];
		if(!playerNode) continue;

		RigidBody* body = playerNode->GetComponent<RigidBody>();
		const Controls& controls = connection->GetControls();
		Quaternion rotation(0.0f, controls.yaw_, 0.0f);

		if(controls.buttons_ & CTRL_FORWARD) n_sub->Translate(Vector3::FORWARD * 1.0f);
		if (controls.buttons_ & CTRL_LEFT) n_sub->Translate(Vector3::LEFT * 1.0f);
		if (controls.buttons_ & CTRL_RIGHT) n_sub->Translate(Vector3::RIGHT * 1.0f);
		if (controls.buttons_ & CTRL_BACK) n_sub->Translate(Vector3::BACK * 1.0f);
	}
}

void CharacterDemo::handleCustomEvent(StringHash eventType, VariantMap& eventData) { 
	
}

void CharacterDemo::handleServerToClientObjectID(StringHash eventType, VariantMap& eventData) { 
	clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	printf("ClientID: %i", clientObjectID_);
}

void CharacterDemo::handleConnectedToServer(StringHash eventType, VariantMap& eventData) {
	std::cout << "Connected to server!!!!!!!!1" << std::endl;
} 

void CharacterDemo::handleClientSceneLoaded(StringHash eventType, VariantMap& eventData) {
	std::cout << "Client loaded scene!" << std::endl;
	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	Node* newObject = CreateCharacter();
	serverObjects_[newConnection] = newObject;

	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = newObject->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}