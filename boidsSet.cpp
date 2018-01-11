#include "boidsSet.h"

boidsSet::boidsSet() {

}

boidsSet::~boidsSet() {

}

void boidsSet::Initialise(ResourceCache* pRc, Scene* pSc) {
	int total_boids = 0;
	boidsLeft = 0;
	for (int i = 1; i <= 20; ++i) {
		for (int j = 0; j < 20; ++j) {
			total_boids++;
			boidList.push_back(boids());
			boidList[total_boids - 1].Initialise(pRc, pSc, i, j, total_boids);
			boidsLeft++;
		}
	}
}
void boidsSet::InitialiseShark(ResourceCache* pRc, Scene* pSc) {
	for (int i = 0; i < 5; i++) {
		sharkList.push_back(shark());
		sharkList[i].Initialise(pRc, pSc);
	}
}

void boidsSet::Update(float delta, float updateCount) {
	if (updateCount == 0) {
		for (int i = 0; i < boidList.size() / 2; ++i) {
			boidList[i].ComputeForce(&boidList[0]);
			boidList[i].Update(delta);
		}
	} else {
		for (int i = boidList.size() / 2; i < boidList.size(); ++i) {
			boidList[i].ComputeForce(&boidList[0]);
			boidList[i].Update(delta);
		}
	}
}

void boidsSet::UpdateShark(float delta) {
	for (int i = 0; i < sharkList.size(); i++) {
		sharkList[i].ComputeForce(&sharkList[0]);
		sharkList[i].Update(delta);
		
	}
	/*RegenShark(delta);*/
}

//void boidsSet::RegenShark(float delta) {
//
//	sharkRegenCounter -= delta;
//
//	if (sharkRegenCounter <= 0.0f) {
//		std::cout << "shark timer ran out!" << std::endl;
//		if (sharksLeft < 5) {
//			std::cout << "less than 5 sharks!" << std::endl;
//			int sharksToRegen = 5 - sharksLeft;
//			for (int i = 0; i < sharksToRegen; i++) {
//				std::cout << "created new shark!" << std::endl;
//				sharkList.push_back(shark());
//				sharksLeft++;
//				sharkList[i].Initialise(cache, scene);
//			}
//			sharkRegenCounter = 20.0f;
//		} else {
//			sharkRegenCounter = 20.0f;
//		}
//	}
//}