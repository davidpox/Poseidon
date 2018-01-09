#include "boidsSet.h"

boidsSet::boidsSet() {

}

boidsSet::~boidsSet() {

}

void boidsSet::Initialise(ResourceCache* pRc, Scene* pSc) {
	int total_boids = 0;
	boidsLeft = 0;
	for (int i = 1; i <= 20; ++i) {
		for (int j = 0; j < 10; ++j) {
			total_boids++;
			boidList.push_back(boids());
			boidList[total_boids - 1].Initialise(pRc, pSc, i, j, total_boids);
			boidsLeft++;
		}
	}
}

void boidsSet::Update(float delta) {
	for (int i = 0; i < boidList.size(); ++i) {
		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(delta);
	}
}

void boidsSet::DeleteBoid(boids boid) {
	for (int i = 0; i < boidList.size(); ++i) {
		if (boidList[i].pNode->GetName() == boid.pNode->GetName()) {
			//boidList.erase(std::remove(boidList.begin, boidList.end, i), boidList.end);
		}
	}
}