#pragma once
#include <array>
#include <iostream>
#include "math.h"
#include <GL/freeglut.h>
class Boat
{
	int lastTime = 0;
public:
	std::array<float, 3> dir = { 0.0f, 0.0f, 1.0f };
	std::array<float, 3> pos = { 0.0f, 0.0f, 0.0f };
	float speed = 0.0f;
	float angle = 0.0f;

	float acceleration = 0.0f;
	float decelerationRate = 1.5f;
	float paddleStrength = 0.8f;
	int directionModifier = 1;

	void rotate(float deg) {
		float rad = deg * (3.14159265f / 180.0f);;

		float dirX = this->dir[0];
		float dirZ = this->dir[2];

		float cosAngle = cos(rad);
		float sinAngle = sin(rad);

		float newDirX = dirX * cosAngle - dirZ * sinAngle;
		float newDirZ = dirX * sinAngle + dirZ * cosAngle;

		this->dir[0] = newDirX;
		this->dir[2] = newDirZ;

		float length = sqrt(newDirX * newDirX + newDirZ * newDirZ);
		if (length != 0.0f) {
			this->dir[0] /= length;
			this->dir[2] /= length;
		}
	}

	void accelerate() {
		this->acceleration += paddleStrength * directionModifier;
	}

	void updateBoatMovement() {
		int currentTime = glutGet(GLUT_ELAPSED_TIME);
		float deltaTime = (currentTime - this->lastTime) / 1000.0f;
		this->lastTime = currentTime;
		// Update speed with acceleration
		this->speed += this->acceleration * deltaTime;

		if (this->acceleration > 0.0f) {
			this->acceleration -= this->decelerationRate * deltaTime;
			if (this->acceleration < 0.0f) this->acceleration = 0.0f;
		}
		else if (this->acceleration < 0.0f) {
			this->acceleration += this->decelerationRate * deltaTime;
			if (this->acceleration > 0.0f) this->acceleration = 0.0f;
		}

		// Limit maximum acceleration
		float maxAcceleration = 3.0f;
		if (this->acceleration > maxAcceleration) this->acceleration = maxAcceleration;
		if (this->acceleration < -maxAcceleration) this->acceleration = -maxAcceleration;

		// Apply deceleration when not paddling
		if (this->speed > 0.0f) {
			this->speed -= this->decelerationRate * deltaTime;
			if (this->speed < 0.0f) this->speed = 0.0f;
		}
		else if (this->speed < 0.0f) {
			this->speed += this->decelerationRate * deltaTime;
			if (this->speed > 0.0f) this->speed = 0.0f;
		}

		// Limit maximum speed
		float maxSpeed = 3.0f;
		if (this->speed > maxSpeed) this->speed = maxSpeed;
		if (this->speed < -maxSpeed) this->speed = -maxSpeed;

		// Update position based on speed and direction
		this->pos[0] += this->dir[0] * this->speed * deltaTime;
		this->pos[2] += this->dir[2] * this->speed * deltaTime;

		/*std::cout << deltaTime << std::endl;
		std::cout << this->acceleration << std::endl;
		std::cout << this->speed << std::endl;
		std::cout << std::endl;
		std::cout << this->dir[0] << std::endl;
		std::cout << this->dir[2] << std::endl;
		std::cout << this->pos[0] << std::endl;
		std::cout << this->pos[2] << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;*/
	}

	void updateBoatRotationAngle() {
		// Calculate the angle between the boat's direction vector and the positive Z-axis
		this->angle = atan2(this->dir[0], this->dir[2]) * (180.0f / 3.14159265f);
	}
};

