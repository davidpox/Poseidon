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
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Engine/Console.h>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include <Math.h>

#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D\Urho2D\AnimatedSprite2D.h>
#include <Urho3D\Urho2D\AnimationSet2D.h>

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
	drawDebug_(false),
	uiRoot_(GetSubsystem<UI>()->GetRoot())
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
	CreateConsoleAndDebugHud();
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
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	camera->SetFarClip(750.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	// ZONE & FOG CREATION
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.09f, 0.301f, 0.647f));
	zone->SetFogStart(1.0f);
	zone->SetFogEnd(90.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));

	Node* n_skybox = scene_->CreateChild("Skybox");
	Skybox* s_skybox = n_skybox->CreateComponent<Skybox>();
	s_skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	s_skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	Node* newPlayer = CreateCharacter();
	cameraNode_->SetPosition(newPlayer->GetPosition() + Vector3(0.0f, 2.0f, 0.0f));
	cameraNode_->SetParent(newPlayer);

	sS.InitialiseShark(cache, scene_);
	bS.Initialise(cache, scene_);

	CreateUI();
	CreateEnvironemnt();
}

void CharacterDemo::CreateClientScene() {
	// SCENE CREATION
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);
	scene_->CreateComponent<DebugRenderer>(LOCAL);

	// CAMERA CREATION
	cameraNode_ = scene_->CreateChild("SpectatorCamera");
	Camera* camera = cameraNode_->CreateComponent<Camera>();
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

		Node* newPlayer = CreateCharacter();
		cameraNode_->SetPosition(newPlayer->GetPosition() + Vector3(0.0f, 2.0f, 0.0f));
		cameraNode_->SetParent(newPlayer);

		sS.InitialiseShark(cache, scene_);
		bS.Initialise(cache, scene_);
	}
	if (gs == CLIENT) {
		Node* playerNode = scene_->GetChild("Player", false);
		if (playerNode) {
			std::cout << "hooked cam to player" << std::endl;
			cameraNode_->SetParent(playerNode);
		} else {
			std::cout << "FAILED to hook cam to player" << std::endl;
		}
	}
	CreateEnvironemnt();
	CreateUI();
}

Node* CharacterDemo::CreateCharacter()
{
	Node* n_sub = scene_->CreateChild("Player");
	n_sub->SetVar("health", 100);
	n_sub->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	n_sub->SetScale(Vector3(0.4f, 0.4f, 0.4f));
	StaticModel* m_sub = n_sub->CreateComponent<StaticModel>();
	m_sub->SetModel(cache->GetResource<Model>("Models/Submarine.mdl"));
	m_sub->ApplyMaterialList();
	RigidBody* rb_sub = n_sub->CreateComponent<RigidBody>();
	rb_sub->SetCollisionLayer(4);
	CollisionShape* cs_sub = n_sub->CreateComponent<CollisionShape>();
	cs_sub->SetBox(Vector3(17.0f, 12.0f, 45.0f));
	cs_sub->SetPosition(Vector3(0.0f, 0.0f, -3.0f));

	Node* nSubHook = n_sub->CreateChild("PlayerHook");
	nSubHook->SetPosition(Vector3(0.0f, 3.0f, -0.5f));
	nSubHook->SetScale(Vector3(0.4f, 0.4f, 0.4f));
	StaticModel* mSubHook = nSubHook->CreateComponent<StaticModel>();
	mSubHook->SetModel(cache->GetResource<Model>("Models/Hook.mdl"));
	mSubHook->SetMaterial(cache->GetResource<Material>("Materials/plane-grey.xml"));
	RigidBody* rbSubHook = nSubHook->CreateComponent<RigidBody>();
	rbSubHook->SetCollisionLayer(2);
	rbSubHook->SetUseGravity(false);
	rbSubHook->SetTrigger(true);
	CollisionShape* csSubHook = nSubHook->CreateComponent<CollisionShape>();
	csSubHook->SetBox(Vector3(10.0f, 10.0f, 1.0f));
	csSubHook->SetPosition(Vector3(0.0f, 5.0f, 23.0f));

	playerNodeID = n_sub->GetID();

	// CREATE PLAYER LIGHT
	Node* n_flashlight = n_sub->CreateChild("Flashlight");
	n_flashlight->SetDirection(cameraNode_->GetDirection());
	Light* l_flashlight = n_flashlight->CreateComponent<Light>();
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
	
	//// CREATE TERRAIN
	//Node* n_terrain = scene_->CreateChild("Terrrain", LOCAL);
	//n_terrain->SetPosition(Vector3(0.0f, -5.0f, 0.0f));
	//t_terrain = n_terrain->CreateComponent<Terrain>();
	//t_terrain->SetSpacing(Vector3(0.4f, 0.05f, 0.2f));
	////t_terrain->SetSmoothing(true);
	//t_terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
	//t_terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	//t_terrain->SetPatchSize(64);
	//t_terrain->SetCastShadows(false);
	////t_terrain->SetOccluder(true);
	//RigidBody* rb_terrain = n_terrain->CreateComponent<RigidBody>();
	//rb_terrain->SetCollisionLayer(2);
	//CollisionShape* cs_terrain = n_terrain->CreateComponent<CollisionShape>();
	//cs_terrain->SetTerrain();

	Node* nBox = scene_->CreateChild("terrain", LOCAL);
	nBox->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	nBox->SetScale(Vector3(409.6f, 1.0f, 204.8f));
	StaticModel* mBox = nBox->CreateComponent<StaticModel>();
	mBox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	mBox->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	RigidBody* rbBox = nBox->CreateComponent<RigidBody>();
	rbBox->SetCollisionLayer(2);
	CollisionShape* csBox = nBox->CreateComponent<CollisionShape>();
	csBox->SetBox(Vector3::ONE);

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
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();
	if (GetSubsystem<UI>()->GetFocusElement()) return;
	Input* input = GetSubsystem<Input>();
	const float MOUSE_SENSITIVITY = 0.1f;
	IntVector2 mouseMove = input->GetMouseMove();
	UI* ui = GetSubsystem<UI>();

	fpsUpdateCounter -= timeStep;

	if (fpsUpdateCounter <= 0.0f) {
		Text* counter = (Text*)uiRoot_->GetChild("fpscounter", true);
		FrameInfo frameInfo = GetSubsystem<Renderer>()->GetFrameInfo();
		counter->SetText("FPS: " + String(ceil(1.0 / frameInfo.timeStep_)));
		//Log::WriteRaw(String(ceil(1.0 / frameInfo.timeStep_ )) + "\n");
		fpsUpdateCounter = 1.0f;
	}
	if (!ui->GetCursor()->IsVisible() && scene_ != nullptr && (gs != NONE && gs != WON && gs != LOST && gs != ENDED)) {
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);
		MOVE_SPEED = 20.0f;

		if (gs == SINGLEPLAYER || gs == SERVER || gs == CLIENT) {
			if(!menuVisible) {
				if(countdowntimer >= 0.0f) {
					countdowntimer -= timeStep;
					String min = (String)((int)countdowntimer / 60 );
					String sec = (String)((int)countdowntimer % 60);
					if ((int)countdowntimer % 60 < 10) timerText->SetText("0" + min + ":0" + sec);
					else timerText->SetText("0" + min + ":" + sec);
				}
				if (countdowntimer <= 0.0f)  {
					gs = ENDED;
					causeofdeath = "The time ran out!";
				}
			}

			if (gs != CLIENT) {
				Node* player = scene_->GetChild("Player", true);
				player->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
				if (input->GetKeyDown(KEY_SHIFT)) MOVE_SPEED *= 10.0f;
				if (input->GetKeyDown(KEY_W)) player->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
				if (input->GetKeyDown(KEY_S)) player->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
				if (input->GetKeyDown(KEY_A)) player->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
				if (input->GetKeyDown(KEY_D)) player->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
				if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
					spawnMissle();
				}
				Vector3 pos = player->GetPosition();
				if ((pos.y_) < 0.0f) {
					player->SetPosition(Vector3(pos.x_, pos.y_ + 1.0f, pos.z_));
				}
				if ((pos.x_ - 2.0f) > 204.8f) {
					player->SetPosition(Vector3(pos.x_ - 2.0f, pos.y_, pos.z_));
				}
				if ((pos.x_ - 2.0f) < -204.8f) {
					player->SetPosition(Vector3(pos.x_ + 2.0f, pos.y_, pos.z_));
				}
				if ((pos.z_ - 2.0f) > 102.4f) {
					player->SetPosition(Vector3(pos.x_, pos.y_, pos.z_ - 2.0f));
				}
				if ((pos.z_ - 2.0f) < -102.4f) {
					player->SetPosition(Vector3(pos.x_, pos.y_, pos.z_ + 2.0f));
				}
				if ((pos.y_ + 2.0f) > 90.0f) {
					player->SetPosition(Vector3(pos.x_, pos.y_ - 2.0f, pos.z_));
				}
			} else {
				cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
			}
		}		

		if (input->GetKeyPress(KEY_P)) {
			drawDebug_ = !drawDebug_;
		}
		if (input->GetKeyPress(KEY_L) && gs != SERVER) {
			Light* playerLight = scene_->GetNode(playerNodeID)->GetChild("Flashlight")->GetComponent<Light>();
			playerLight->SetEnabled(!playerLight->IsEnabled());
		}
		if (gs != CLIENT) {
			bS.Update(timeStep, updateCount);
			if (updateCount == 0) updateCount++;
			else updateCount--;
			sS.UpdateShark(timeStep);
		}

		if (gs == SERVER || gs == SINGLEPLAYER) {
			if (uiRoot_->GetChild("warningText", false)->IsVisible()) warningTextCounter += timeStep;

			if (warningTextCounter >= 5.0f) {
				uiRoot_->GetChild("warningText", false)->SetVisible(false);
				warningTextCounter = 0.0f;
			}
		}
	}
	
	if (input->GetKeyPress(KEY_M) && gs != NONE && gs != WON && gs != LOST && gs != ENDED) {
		menuVisible = !menuVisible;
		if (gs == PAUSED) gs = t;
		else  {
			t = gs;
			gs = PAUSED;
		}
		ui->GetCursor()->SetVisible(menuVisible);
		window_->SetVisible(menuVisible);
	}

	if (gs == SERVER) {
		Network* network = GetSubsystem<Network>();
		const Vector<SharedPtr<Connection>>& connections = network->GetClientConnections();
		for (unsigned i = 0; i < connections.Size(); ++i) {
			Connection* connection = connections[i];

			Node* playerNode = serverObjects_[connection];
			if (!playerNode) continue;

			const Controls& controls = connection->GetControls();
			Quaternion rotation(controls.pitch_, controls.yaw_, 0.0f);


			//if (controls.buttons_ & CTRL_FORWARD) playerNode->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
			//if (controls.buttons_ & CTRL_LEFT) playerNode->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
			//if (controls.buttons_ & CTRL_RIGHT) playerNode->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
			//if (controls.buttons_ & CTRL_BACK) playerNode->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
		}
	}
}

void CharacterDemo::HandleCollision(StringHash eventType, VariantMap& eventData) {
	if (gs == SINGLEPLAYER || gs == SERVER) {
		using namespace NodeCollision;
		auto* collided = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
		auto* collided2 = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());

		if (collided2->GetNode()->GetName() == "Missile") {
			collided2->GetNode()->Remove();
			missileCount--;
			if (collided->GetNode()->GetName().Contains("Boid_", false)) {
				Node* player = scene_->GetNode(playerNodeID);
				int health = player->GetVar("health").GetInt();
				collided->GetNode()->SetEnabled(false);
				if (health > 0) {
					uiRoot_->GetChild("warningText", false)->SetVisible(true);
					fishKilled++;
					bS.boidsLeft--;
					health -= 5;
					if (health <= 0) causeofdeath = "YOU KILLED TOO MANY FISHIES :(\nHOW ARE YOU GOING TO FEED YOUR FAMILY?";
					player->SetVar("health", health);
				}
			}
			if (collided->GetNode()->GetName().Contains("Shark", false)) {
				int health = collided->GetNode()->GetVar("health").GetInt();
				if (health > 0) {
					health -= 10;
					collided->GetNode()->SetVar("health", health);
				} else {
					collided->GetNode()->SetEnabled(false);
				}
			}
		}

		if (collided->GetNode()->GetName() == "PlayerHook") {
			if (collided2->GetNode()->GetName().Contains("Boid_", false)) {
				collided2->GetNode()->SetEnabled(false);
				fishCaught++;
				bS.boidsLeft--;
			}
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
		if (gs == SINGLEPLAYER || gs == SERVER) {
			Node* player = scene_->GetChild("Player", true);
			uiRoot_->GetChild("playerHealthBar", false)->SetWidth(player->GetVar("health").GetInt() * 2);
			Text* healthText = (Text*)uiRoot_->GetChild("playerHealthText", false);
			healthText->SetText((String)player->GetVar("health").GetInt() + "HP");

			Text* fishKilledText = (Text*)uiRoot_->GetChild("fishKilledText", false);
			fishKilledText->SetText((String)fishKilled);

			Text* fishCaughtText = (Text*)uiRoot_->GetChild("fishCaughtText", false);
			fishCaughtText->SetText((String)fishCaught);

			if (player->GetVar("health").GetInt() <= 0) gs = LOST;
			if (bS.boidsLeft <= 0) gs = WON;
		}
		if (gs == ENDED || gs == WON || gs == LOST) {
			UI* ui = GetSubsystem<UI>();
			ui->Clear();
			scene_->Clear(true, true);
			CreateEndScreen();
		}
	}
}

void CharacterDemo::spawnMissle() {
	if (missileCount < MAX_MISSLES) {
		missileCount++;
		Node* player = scene_->GetNode(playerNodeID);
		missileLeftOrRight++;
		missle t = missle();
		t.Initialise(cache, scene_);
		
		t.pRigidBody->SetRotation(player->GetRotation());
		if (missileLeftOrRight == 1) {
			t.pRigidBody->SetPosition((player->GetPosition() + (player->GetRotation() * Vector3(-4.0f, -0.6f, 9.0f))));
		} else {
			t.pRigidBody->SetPosition((player->GetPosition() + (player->GetRotation() * Vector3(4.0f, -0.6f, 9.0f))));
			missileLeftOrRight = 0;
		}
		t.pRigidBody->SetLinearVelocity(player->GetDirection() * 100.0f);
	}
}

void CharacterDemo::CreateMainMenu() {
	Graphics* graphics = GetSubsystem<Graphics>();
	UI* ui = GetSubsystem<UI>();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	uiRoot_->SetDefaultStyle(uiStyle);
	float winWidth = (float)graphics->GetWidth();
	float winHeight = (float)graphics->GetHeight();

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);

	window_ = new Window(context_);
	uiRoot_->AddChild(window_);

	Texture2D* background = cache->GetResource<Texture2D>("Textures/background1.jpg");
	Sprite* sprite = uiRoot_->CreateChild<Sprite>();
	sprite->SetName("background-texture");
	sprite->SetTexture(background);
	sprite->SetHotSpot(0, 0);
	sprite->SetSize(1024, 576);
	sprite->SetPriority(-100);

	Texture2D* logo = cache->GetResource<Texture2D>("Textures/PoseidonLogo.png");
	Sprite* logosprite = uiRoot_->CreateChild<Sprite>();
	logosprite->SetName("poseidonlogo");
	logosprite->SetTexture(logo);
	logosprite->SetHotSpot(80, 100);
	logosprite->SetSize(160, 200);
	logosprite->SetPosition(winWidth /2, (logosprite->GetHeight() / 2) + 20);

	Font* font = cache->GetResource<Font>("Fonts/Roboto-Thin.TTF");
	window_->SetMinWidth(200);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("Window");
	window_->SetStyleAuto();

	window_->SetPosition(0, 100);

	Text* fpscounter = uiRoot_->CreateChild<Text>();
	fpscounter->SetName("fpscounter");
	fpscounter->SetText("FPS: 0");
	fpscounter->SetFont(font, 20);
	fpscounter->SetColor(Color::GREEN);
	fpscounter->SetPosition(0, 0);
	fpscounter->SetPriority(1000);
	

	Text* menuText = menu_->CreateText("MAIN MENU", 16, window_, font, 16);
	menuText->SetColor(Color::WHITE);
	Button* btnSingleplayer = menu_->CreateButton("SINGLEPLAYER", 24, window_, font);
	btnSingleplayer->SetName("bSingleplayer");
	
	Text* serverText = menu_->CreateText("SERVER", 16, window_, font, 16);
	serverText->SetColor(Color::WHITE);
	serverText->SetName("serverTitle");
	leIPAddress = menu_->CreateLineEdit("", 24, window_, font);
	leIPAddress->SetName("ipLE");
	Button* btnConnect = menu_->CreateButton("CONNECT", 24, window_, font);
	btnConnect->SetName("bConnect");
	Button* btnCreateServer = menu_->CreateButton("START SERVER", 24, window_, font);
	btnCreateServer->SetName("bCreateServer");
	Text* spacer1 = menu_->CreateText("", 16, window_, font, 16);
	spacer1->SetName("spacer1");
	Button* btnDisconnect = menu_->CreateButton("DISCONNECT", 24, window_, font);
	btnDisconnect->SetName("bDisconnect");
	Button* btnQuit = menu_->CreateButton("QUIT", 24, window_, font);
	//ToolTip* tp = menu_->CreateToolTip("Exits the application", font, 12, window_);
	//QuitButton->AddChild(tp);

	SubscribeToEvent(btnQuit, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleQuit));
	SubscribeToEvent(btnSingleplayer, E_RELEASED, URHO3D_HANDLER(CharacterDemo, StartSingleplayer));
	SubscribeToEvent(btnConnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleConnect));
	SubscribeToEvent(btnCreateServer, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleCreateServer));
	SubscribeToEvent(btnDisconnect, E_RELEASED, URHO3D_HANDLER(CharacterDemo, handleDisconnect));
	window_->SetVisible(menuVisible);
}

void CharacterDemo::HandleQuit(StringHash eventType, VariantMap& eventData) {
	engine_->Exit();
}

void CharacterDemo::StartSingleplayer(StringHash eventType, VariantMap& eventData) {
	UI* ui = GetSubsystem<UI>();
	gs = SINGLEPLAYER;
	{
		uiRoot_->RemoveChild(uiRoot_->GetChild("background-texture", false));
		uiRoot_->RemoveChild(uiRoot_->GetChild("poseidonlogo", false));
		window_->GetChild("bSingleplayer", false)->Remove();
		window_->GetChild("serverTitle", false)->Remove();
		window_->GetChild("ipLE", false)->Remove();
		window_->GetChild("bConnect", false)->Remove();
		window_->GetChild("bCreateServer", false)->Remove();
		window_->GetChild("spacer1", false)->Remove();
		window_->GetChild("bDisconnect", false)->Remove();
		window_->SetPosition(0, 0);
		window_->SetHeight(50);
	}

	fishKilled = 0;
	fishCaught = 0;
	countdowntimer = 300.0f;
	CreateClientScene();
	menuVisible = !menuVisible;
	window_->SetVisible(false);
	ui->GetCursor()->SetVisible(false);
}

void CharacterDemo::handleConnect(StringHash eventType, VariantMap& eventData) {
	gs = CLIENT;
	{
		uiRoot_->RemoveChild(uiRoot_->GetChild("background-texture", false));
		uiRoot_->RemoveChild(uiRoot_->GetChild("poseidonlogo", false));
		window_->GetChild("bSingleplayer", false)->Remove();
		window_->GetChild("serverTitle", false)->Remove();
		window_->GetChild("ipLE", false)->Remove();
		window_->GetChild("bConnect", false)->Remove();
		window_->GetChild("bCreateServer", false)->Remove();
		window_->GetChild("spacer1", false)->Remove();
		window_->GetChild("bDisconnect", false)->Remove();
		window_->SetPosition(0, 0);
		window_->SetHeight(50);
	}
	CreateClientScene();
	Network* network = GetSubsystem<Network>();
	String address = leIPAddress->GetText().Trimmed();
	if (address.Empty()) address = "localhost";
	network->Connect(address, SERVER_PORT, scene_);
}

void CharacterDemo::handleCreateServer(StringHash eventType, VariantMap& eventData) {
	UI* ui = GetSubsystem<UI>();
	{
		uiRoot_->RemoveChild(uiRoot_->GetChild("background-texture", false));
		uiRoot_->RemoveChild(uiRoot_->GetChild("poseidonlogo", false));
		window_->GetChild("bSingleplayer", false)->Remove();
		window_->GetChild("serverTitle", false)->Remove();
		window_->GetChild("ipLE", false)->Remove();
		window_->GetChild("bConnect", false)->Remove();
		window_->GetChild("bCreateServer", false)->Remove();
		window_->GetChild("spacer1", false)->Remove();
		window_->GetChild("bDisconnect", false)->Remove();
		window_->SetPosition(0, 0);
		window_->SetHeight(50);
	}
	gs = SERVER;
	fishKilled = 0;
	fishCaught = 0;
	countdowntimer = 300.0f;
	CreateServerScene();
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);
	menuVisible = !menuVisible;
	window_->SetVisible(false);
	ui->GetCursor()->SetVisible(false);
}

void CharacterDemo::handleDisconnect(StringHash eventType, VariantMap& eventData) {
	Network* network = GetSubsystem<Network>();
	Connection* connection = network->GetServerConnection();

	if (connection) {
		connection->Disconnect();
		scene_->Clear(true, false);
		clientObjectID_ = 0;
		gs = NONE;
	} else if (network->IsServerRunning()) {
		network->StopServer();
		scene_->Clear(true, false);
		gs = NONE;
	}
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
	}
}

Controls CharacterDemo::FromClientToServerControls() {
	Input* input = GetSubsystem<Input>();
	Controls controls;
	//controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	//controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	//controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	//controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.pitch_ = pitch_;
	controls.yaw_ = yaw_;
	return controls;
}

void CharacterDemo::handleCustomEvent(StringHash eventType, VariantMap& eventData) { 
	
}

void CharacterDemo::handleServerToClientObjectID(StringHash eventType, VariantMap& eventData) { 
	//clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	//printf("ClientID: %i", clientObjectID_);
}

void CharacterDemo::handleConnectedToServer(StringHash eventType, VariantMap& eventData) {
	
} 

void CharacterDemo::handleClientSceneLoaded(StringHash eventType, VariantMap& eventData) {
	std::cout << "Client loaded scene!" << std::endl;
	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());



	//Node* newObject = createSpectator();
	//serverObjects_[newConnection] = newObject;

	//VariantMap remoteEventData;
	//remoteEventData[PLAYER_ID] = newObject->GetID();
	//newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);
}

void CharacterDemo::CreateUI() {
	UI* ui = GetSubsystem<UI>();
	Graphics* graphics = GetSubsystem<Graphics>();
	Font* font = cache->GetResource<Font>("Fonts/Roboto-Thin.ttf");
	float winWidth = (float)graphics->GetWidth();
	float winHeight = (float)graphics->GetHeight();

	Texture2D* tex = cache->GetResource<Texture2D>("Textures/crosshair.png");
	Sprite* sprite = uiRoot_->CreateChild<Sprite>();
	sprite->SetName("name");
	sprite->SetTexture(tex);
	sprite->SetHotSpot(7, 7);
	sprite->SetSize(15, 15);
	sprite->SetPosition(Vector2(winWidth/2, winHeight/2));

	Texture2D* healthbarTex = cache->GetResource<Texture2D>("Textures/red.png");
	Sprite* healthSprite = uiRoot_->CreateChild<Sprite>();
	healthSprite->SetName("playerHealthBar");
	healthSprite->SetTexture(healthbarTex);
	healthSprite->SetSize(200, 40);
	healthSprite->SetHotSpot(100, 20);
	healthSprite->SetPosition(110, winHeight - 30);

	Text* healthText = uiRoot_->CreateChild<Text>();
	healthText->SetName("playerHealthText");
	healthText->SetText("100HP");
	healthText->SetFont(font, 20);
	healthText->SetColor(Color::WHITE);
	healthText->SetPosition(healthSprite->GetPosition().x_ - (healthText->GetWidth() / 2), healthSprite->GetPosition().y_ - (healthText->GetHeight() / 2));
	healthText->SetPriority(10);

	Text* warningText = uiRoot_->CreateChild<Text>();
	warningText->SetName("warningText");
	warningText->SetText("You're supposed to catch the fish, not kill them!");
	warningText->SetFont(font, 16);
	warningText->SetColor(Color::RED);
	warningText->SetPosition(winWidth - warningText->GetWidth() - 10, winHeight - 30);
	warningText->SetVisible(false);

	Texture2D* deadFishTex = cache->GetResource<Texture2D>("Textures/iconDeadFish.png");
	Sprite* spriteDeadFish = uiRoot_->CreateChild<Sprite>();
	spriteDeadFish->SetName("deadFishIcon");
	spriteDeadFish->SetTexture(deadFishTex);
	spriteDeadFish->SetSize(50,50);
	spriteDeadFish->SetHotSpot(25, 25);
	spriteDeadFish->SetPosition(35, healthSprite->GetPosition().y_ - 50);

	Text* fishKilledText = uiRoot_->CreateChild<Text>();
	fishKilledText->SetName("fishKilledText");
	fishKilledText->SetText("0");
	fishKilledText->SetFont(font, 28);
	fishKilledText->SetColor(Color::WHITE);
	fishKilledText->SetPosition(70, healthSprite->GetPosition().y_ - 70);

	Texture2D* caughtFishTex = cache->GetResource<Texture2D>("Textures/iconNet.png");
	Sprite* spriteCaughtFish = uiRoot_->CreateChild<Sprite>();
	spriteCaughtFish->SetName("caughtFishIcon");
	spriteCaughtFish->SetTexture(caughtFishTex);
	spriteCaughtFish->SetSize(50, 50);
	spriteCaughtFish->SetHotSpot(25, 25);
	spriteCaughtFish->SetPosition(120, healthSprite->GetPosition().y_ - 50);

	Text* fishCaughtText = uiRoot_->CreateChild<Text>();
	fishCaughtText->SetName("fishCaughtText");
	fishCaughtText->SetText("0");
	fishCaughtText->SetFont(font, 28);
	fishCaughtText->SetColor(Color::WHITE);
	fishCaughtText->SetPosition(155, healthSprite->GetPosition().y_ - 70);

	timerText = uiRoot_->CreateChild<Text>();
	timerText->SetName("timerText");
	timerText->SetText("00:00");
	timerText->SetFont(font, 32);
	timerText->SetColor(Color::WHITE);
	timerText->SetPosition((winWidth / 2) - (timerText->GetWidth() / 2), 20);
}

void CharacterDemo::CreateEndScreen() {
	UI* ui = GetSubsystem<UI>();
	Graphics* graphics = GetSubsystem<Graphics>();
	//XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	Font* font = cache->GetResource<Font>("Fonts/Roboto-Thin.ttf");
	float winWidth = (float)graphics->GetWidth();
	float winHeight = (float)graphics->GetHeight();

	ui->GetCursor()->SetVisible(true);

	Texture2D* background = cache->GetResource<Texture2D>("Textures/background1.jpg");
	Sprite* sprite = uiRoot_->CreateChild<Sprite>();
	sprite->SetName("background-texture");
	sprite->SetTexture(background);
	sprite->SetHotSpot(0, 0);
	sprite->SetSize(1024, 576);
	sprite->SetPriority(-100);

	Texture2D* tridentTexture = cache->GetResource<Texture2D>("Textures/TridentFull.png");
	Sprite* tridentSprite = uiRoot_->CreateChild<Sprite>();
	tridentSprite->SetName("name");
	tridentSprite->SetTexture(tridentTexture);
	tridentSprite->SetHotSpot(60, 60);
	tridentSprite->SetSize(120, 120);
	tridentSprite->SetPosition(Vector2(winWidth / 2, 130));

	Text* endedTitleText = uiRoot_->CreateChild<Text>();
	endedTitleText->SetName("endedTitle");
	if (gs == WON) endedTitleText->SetText("YOU WON!");
	if (gs == LOST) endedTitleText->SetText("YOU LOST!"); 
	if (gs == ENDED) endedTitleText->SetText("GAME ENDED!");
	endedTitleText->SetFont(font, 32);
	endedTitleText->SetColor(Color::WHITE);
	endedTitleText->SetPosition((winWidth / 2) - (endedTitleText->GetWidth() / 2), tridentSprite->GetPosition().y_ + 70);

	Text* causeofdeathText = uiRoot_->CreateChild<Text>();
	causeofdeathText->SetName("endedTitle");
	causeofdeathText->SetText(causeofdeath);
	causeofdeathText->SetFont(font, 20);
	causeofdeathText->SetColor(Color::WHITE);
	causeofdeathText->SetPosition((winWidth / 2) - (causeofdeathText->GetWidth() / 2), endedTitleText->GetPosition().y_ + 50);
	causeofdeathText->SetTextAlignment(HA_CENTER);

	Text* fishesCaughtText = uiRoot_->CreateChild<Text>();
	fishesCaughtText->SetName("endedTitle");
	fishesCaughtText->SetText("You caught: " + (String)fishCaught + " fishes!");
	fishesCaughtText->SetFont(font, 20);
	fishesCaughtText->SetColor(Color::WHITE);
	fishesCaughtText->SetPosition((winWidth / 2) - (fishesCaughtText->GetWidth() / 2), causeofdeathText->GetPosition().y_ + causeofdeathText->GetHeight() + 10);

	Text* fishesKilledText = uiRoot_->CreateChild<Text>();
	fishesKilledText->SetName("endedTitle");
	if (fishKilled > 0) fishesKilledText->SetText("and killed " + (String)fishKilled + " fishes :(");
	else fishesKilledText->SetText("and killed " + (String)fishKilled + " fishes :D!");
	fishesKilledText->SetFont(font, 20);
	fishesKilledText->SetColor(Color::WHITE);
	fishesKilledText->SetPosition((winWidth / 2) - (fishesKilledText->GetWidth() / 2), fishesCaughtText->GetPosition().y_ + 30);
}