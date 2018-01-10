#include "boids.h"

float boids::Range_FAttract = 20.0f;
float boids::Range_FRepel = 3.0f;
float boids::Range_FAlign = 5.0f;
float boids::FAttract_Vmax = 2.0f;
float boids::FAttract_Factor = 4.0f;
float boids::FRepel_Factor = 2.0f;
float boids::FAlign_Factor = 2.0f;

boids::boids() {
}

boids::~boids() {

}

void boids::Initialise(ResourceCache* pRc, Scene* pSc, int group, int boid_group_num, int boidNum) {
	pNode = pSc->CreateChild("Boid_" + (String)group + "-" + (String)boid_group_num);
	pNode->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
	pNode->SetScale(Vector3(10.0f, 10.0f, 10.0f));
	pNode->SetVar("boid_number", boidNum);
	pNode->SetVar("group_number", group);
	pNode->SetVar("boid_num_in_group", boid_group_num);

	group_num = group;
	
	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRc->GetResource<Model>("Models/boid_fish.mdl"));
	pObject->SetMaterial(pRc->GetResource<Material>("Materials/fish_diffmap.xml"));

	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetUseGravity(false);
	pRigidBody->SetMass(1.0f);
	pRigidBody->SetPosition(Vector3(Random(180.0f) - 90.0f, Random(10.0f, 30.0f), Random(180.0f) - 90.0f));
	pRigidBody->SetLinearVelocity(Vector3(Random(-20.0f,20.0f), 0.0f, Random(-20.0f,20.0f)));

	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetBox(Vector3(0.04, 0.1, 0.35));

	
}

void boids::ComputeForce(boids* pBoids) {
	Vector3 CoM;
	int n = 0, b = 0, s = 0;
	force = Vector3(0, 0, 0);
	Vector3 forceAttract, forceAlign, forceSeperation;
	Vector3 steeringForce;
	Vector3 pMean, vMean;

	for (int i = 0; i < NumBoids; i++) {
		if (this == &pBoids[i]) continue;
		if (group_num != pBoids[i].group_num) continue;

		Vector3 sep = pRigidBody->GetPosition() - pBoids[i].pRigidBody->GetPosition();
		float d = sep.Length();
		if (d < Range_FAttract) {
			CoM += pBoids[i].pRigidBody->GetPosition();
			n++;
		}
		if (d < Range_FAlign) {
			vMean += pBoids[i].pRigidBody->GetLinearVelocity();
			b++;
		}
		if (d < Range_FRepel) {
			float f = (sep.Length() / pow(sep.Length(), 2.0));
			forceSeperation += (sep / f);
			s++;
		}
	}

	if (b > 0) {
		vMean /= b;
	}
	if (s > 0) {
		forceSeperation *= FRepel_Factor;
	}
	if (n > 0) {
		CoM /= n;
		forceSeperation *= FRepel_Factor;

		Vector3 dir = (CoM - pRigidBody->GetPosition()).Normalized();
		Vector3 vDesired = dir * FAttract_Vmax;
		forceAttract += (vDesired - pRigidBody->GetLinearVelocity()) * FAttract_Factor;
		forceAlign = FAlign_Factor * (vMean - pRigidBody->GetLinearVelocity());

		force = forceAttract + forceAlign + forceSeperation;
	}
}

void boids::Update(float delta) {
	pRigidBody->ApplyForce(force);

	Vector3 vel = pRigidBody->GetLinearVelocity();
	float d = vel.Length();
	if (d < 10.0f) {
		d = 10.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized() * d);
	}
	else if (d > 50.0f) {
		d = 50.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized() * d);
	}

	Vector3 pos = pRigidBody->GetPosition();
	if (pos.y_ < -10.0f) {
		pos.y_ = 99.0f; // Resets to top if too low
		pRigidBody->SetPosition(pos);
	}
	if (pos.y_ > 100.0f) {
		pos.y_ = 2.0f; // Resets to bottom if too high
		pRigidBody->SetPosition(pos);
	}

	pNode->LookAt(pNode->GetPosition() + -pRigidBody->GetLinearVelocity());	
}