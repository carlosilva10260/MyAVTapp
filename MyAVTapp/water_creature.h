#pragma once
#include <array>
#include "aabb.h"
class WaterCreature
{
	static float generateInitialPos() {
		return rand() % 200 - 100;
	}

	static float generateInitialSpeed() {
		return (rand() % 10 + static_cast<float>(1)) / 5;
	}

	static int generateMoveDir() {
		return (rand() % 2);
	}

	static int generateDirection() {
		return (rand() % 2) == 0 ? 1 : -1;
	}
public:
	std::array<float, 3> pos;
	float speed;
	int moveAxis;
	int direction;
	float spinAngle;
	
	WaterCreature() : 
		pos({generateInitialPos(), 0, generateInitialPos()}),
		speed(generateInitialSpeed()),
		moveAxis(generateMoveDir()),
		direction(generateDirection()), 
		spinAngle(0.0f) {}

	void speedUp(float increment) {
		this->speed += increment;
	}

	void angleUp(float increment) {
		if (this->spinAngle + increment > 360) {
			this->spinAngle = this->spinAngle + increment - 360;
		}
		else {
			this->spinAngle += increment;
		}
	}

	std::pair<float, float>* getRotateAxis(void) const {
		return this->moveAxis == 0 ? new std::pair<float, float>(0.0f, 1.0f) : new std::pair<float, float>(1.0f, 0.0f);
	}

	AABB getCreatureAABB() {
		float halfWidth, halfDepth;

		const auto rotateDirs = getRotateAxis();

		if (rotateDirs->second == 1 && rotateDirs->first == 0) {
			// Cylinder aligned along the x-axis
			halfWidth = 1.0f;  // Height becomes width
			halfDepth = 0.5f;  // Radius remains the depth
		}
		else {
			// Cylinder aligned along the z-axis
			halfWidth = 0.5f;  // Radius remains the width
			halfDepth = 1.0f;  // Height becomes depth
		}

		AABB box{};
		box.min[0] = this->pos[0] - halfWidth;
		box.min[1] = this->pos[1] - 0.5;
		box.min[2] = this->pos[2] - halfDepth;

		box.max[0] = this->pos[0] + halfWidth;
		box.max[1] = this->pos[1] + 0.5;
		box.max[2] = this->pos[2] + halfDepth;

		return box;
	}
};

