#pragma once
#include <array>
#include "aabb.h"
#include <GL/freeglut.h>
#include <stdio.h>
class Redfloat
{
	int lastTime = 0;
	float decelerationRate = 1.5f;
public:
	std::array<float, 3> pos;
	std::array<float, 3> dir = { 0.0f, 0.0f, 0.0f };
	float speed = 0.0f;

	Redfloat() : pos({0.0f, 0.0f, 0.0f}) {}

	Redfloat(std::array<float, 3> &pos) : pos(pos) {}

	Redfloat(float posX, float posY, float posZ) : pos({ posX, posY, posZ }) {}

	AABB getFloatAABB(void) {
		return AABB(pos[0] - 1.0f, pos[1], pos[2] - 1.0f,
			pos[0] + 1.0f, pos[1], pos[2] + 1.0f);
	}

	void updateFloatPos(void) {
		int currentTime = glutGet(GLUT_ELAPSED_TIME);
		float deltaTime = (currentTime - this->lastTime) / 1000.0f;
		this->lastTime = currentTime;
		if (speed == 0.0f) return;

		this->pos[0] += dir[0] * (speed * deltaTime);
		this->pos[2] += dir[2] * (speed * deltaTime);

		if (this->speed > 0.0f) {
			this->speed -= this->decelerationRate * deltaTime;
		}
		if (this->speed < 0.0f) {
			this->speed = 0.0f;
		}
		this->decelerationRate = this->speed;
	}

	void onCollide(std::array<float, 3>& reactionDir, float collisionSpeed) {
		this->dir = reactionDir;
		this->speed = collisionSpeed / 2.0f;
	}
};

