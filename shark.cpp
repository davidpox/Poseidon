#include "shark.h"

float shark::Shark_Range = 75.0f;
float shark::FAttract_Factor = 10.0f;


shark::shark() {
}

shark::~shark() {

}

void shark::Initialise(ResourceCache* pRc, Scene* pSc) {
	nShark = pSc->CreateChild("Shark");
	nShark->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	nShark->SetScale(Vector3(2.0f, 2.0f, 2.0f));
	nShark->SetVar("health", 100);

	StaticModel* mShark = nShark->CreateComponent<StaticModel>();
	mShark->SetModel(pRc->GetResource<Model>("Models/shark.mdl"));
	mShark->ApplyMaterialList();

	rbShark = nShark->CreateComponent<RigidBody>();
	rbShark->SetCollisionLayer(2);
	rbShark->SetUseGravity(false);
	rbShark->SetMass(1.0f);
	rbShark->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(10.0f, 30.0f), Random(180.0f) - 90.0f));
	rbShark->SetLinearVelocity(Vector3(Random(-40.0f, 40.0f), 0.0f, Random(-40.0f, 40.0f)));

	CollisionShape* csShark = nShark->CreateComponent<CollisionShape>();
	csShark->SetBox(Vector3(1.75f, 2.0f, 7.0f));
	csShark->SetPosition(Vector3(-0.2f, 0.5f, 0.0f));

	playerNode = pSc->GetChild("Player", false);
}

void shark::ComputeForce(shark* pShark) {
	Vector3 pos = rbShark->GetPosition();
	Vector3 playerPos = playerNode->GetPosition();
	Vector3 seperation = pos - playerPos;
	Vector3 seperationForce;
	int s = 0;

	for (int i = 0; i < 5; i++) {	
		if (this == &pShark[i]) continue;
		Vector3 sharkSep = rbShark->GetPosition() - pShark[i].rbShark->GetPosition();
		float d = sharkSep.Length();
		if (d < 15.0f) {
			float f = (sharkSep.Length() / pow(sharkSep.Length(), 2.0f));
			seperationForce += (sharkSep / f);
			s++;
		}
	}
	if (s > 0) seperationForce *= 15.0f;

	if (seperation.Length() < Shark_Range) {
		Vector3 direction = ((playerNode->GetPosition() - rbShark->GetPosition()).Normalized()) * 10.0f;
		force = ((direction - rbShark->GetLinearVelocity()) * 10.0f) + seperationForce;
	}
}

void shark::Update(float delta) {
	rbShark->ApplyForce(force);

	attackcooldown -= delta;
	if (attackcooldown <= 0.0f) {
		attack();
		attackcooldown = 5.0f;
	}

	//Vector3 vel = rbShark->GetLinearVelocity();
	//float d = vel.Length();
	//if (d < 10.0f) {
	//	d = 10.0f;
	//	rbShark->SetLinearVelocity(vel.Normalized() * d);
	//} else if (d > 50.0f) {
	//	d = 50.0f;
	//	rbShark->SetLinearVelocity(vel.Normalized() * d);
	//}

	Vector3 pos = rbShark->GetPosition();
	if (pos.y_ < -10.0f) {
		pos.y_ = 99.0f; // Resets to top if too low
		rbShark->SetPosition(pos);
	}
	if (pos.y_ > 100.0f) {
		pos.y_ = 2.0f; // Resets to bottom if too high
		rbShark->SetPosition(pos);
	}

	nShark->LookAt(nShark->GetPosition() + -rbShark->GetLinearVelocity());
}

void shark::attack() {
	Vector3 pos = rbShark->GetPosition();
	Vector3 playerPos = playerNode->GetPosition();
	Vector3 seperation = pos - playerPos;
	if (seperation.Length() < 15.0f) {
		int health = playerNode->GetVar("health").GetInt();
		if (health > 0) {
			health -= 10;
			playerNode->SetVar("health", health);
		} 
	}
}
