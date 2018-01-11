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
#include <algorithm>
#include "boids.h"
#include "shark.h"
#include <vector>

namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
}
// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;



class boidsSet {
public: 
	
	//boids boidList[NumBoids];
	std::vector<boids> boidList;
	std::vector<shark> sharkList;
	int boidsLeft;
	int sharksLeft = 0;
	float sharkRegenCounter = 20.0f;

	ResourceCache* cache;
	Scene* scene;

	boidsSet();
	~boidsSet();
	 
	void Initialise(ResourceCache* pRc, Scene* pSc);
	void InitialiseShark(ResourceCache* pRc, Scene* pSc);
	void Update(float delta, float updateCount);
	void UpdateShark(float delta);
	void RegenShark(float delta);
};