#include "missle.h"


missle::missle() {

}

missle::~missle() {

}

void missle::Initialise(ResourceCache* pRc, Scene* pSc) {
	isDead = false;
	decayTimer = 0.0f;

	pNode = pSc->CreateChild("Missile");
	pNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRc->GetResource<Model>("Models/rocket.mdl"));
	pObject->SetMaterial(pRc->GetResource<Material>("Materials/plane-red.xml"));

	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetCollisionMask(251);
	pRigidBody->SetUseGravity(false);
	pRigidBody->SetMass(0.2f);

	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetBox(Vector3(1.0f, 1.0f, 4.0f));

	pObject->SetEnabled(true);
}