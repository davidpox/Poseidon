#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
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
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <iostream>
#include <set>
#include <Urho3D/IO/Log.h>
#include <Urho3D/DebugNew.h>

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

static int NumSharks = 5;

class shark {
	static float Shark_Range;
	static float FAttract_Factor;


public: 
	Node* nShark;
	Node* playerNode;
	RigidBody* rbShark;
	Vector3 force;
	float attackcooldown = 0.0f;


	shark::shark();
	shark::~shark();

	void Initialise(ResourceCache* pRc, Scene* pSc);
	void ComputeForce(shark* pShark);
	void Update(float delta);
	void attack();
};