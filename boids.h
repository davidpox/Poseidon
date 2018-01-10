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

static int NumBoids = 200;

class boids {

	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;
	static float FAttract_Factor;
	static float FRepel_Factor;
	static float FAlign_Factor;
	static float FAttract_Vmax;
	static int MAX_BOIDS_GROUP_SIZE;

public: 
	Vector3 force;
	Node* pNode;
	RigidBody* pRigidBody;
	CollisionShape* pCollisionShape;
	StaticModel* pObject;
	int group_num; 
	int group_size;


	boids::boids();
	boids::~boids();

	void Initialise(ResourceCache* pRc, Scene* pSc, int group_num, int boid_num, int boidNum);
	void ComputeForce(boids* pBoid);
	void Update(float delta);
};