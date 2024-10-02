#pragma once
#include <array>
#include <iostream>
#include "math.h"
#include "aabb.h"
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
			this->decelerationRate = this->speed / 2;
		}
		else if (this->speed < 0.0f) {
			this->speed += this->decelerationRate * deltaTime;
			if (this->speed > 0.0f) this->speed = 0.0f;
			this->decelerationRate = - this->speed / 2;
		}

		// Limit maximum speed
		float maxSpeed = 20.0f;
		if (this->speed > maxSpeed) this->speed = maxSpeed;
		if (this->speed < -maxSpeed) this->speed = -maxSpeed;

		// Update position based on speed and direction
		this->pos[0] += this->dir[0] * this->speed * deltaTime;
		this->pos[2] += this->dir[2] * this->speed * deltaTime;
	}

	void updateBoatRotationAngle() {
		// Calculate the angle between the boat's direction vector and the positive Z-axis
		this->angle = atan2(this->dir[0], this->dir[2]) * (180.0f / 3.14159265f);
	}

	void addPaddleStrength(float paddleStrength) {
		this->paddleStrength += paddleStrength;
	}

	void stop() {
		// Simulates a crash with rebounding
		this->pos[0] -= this->dir[0] * (this->speed / 5.0f);
		this->pos[2] -= this->dir[2] * (this->speed / 5.0f);

		this->speed = 0.0f;
		this->acceleration = 0.0f;
		this->paddleStrength = 0.8f;
	}

	void resetBoatPosition() {
		this->pos[0] = 0.0f;
		this->pos[1] = 0.0f;
		this->pos[2] = 0.0f;

		this->dir[0] = 0.0f;
		this->dir[1] = 0.0f;
		this->dir[2] = 1.0f;

		this->speed = 0.0f;
		this->angle = 0.0f;
		this->acceleration = 0.0f;
		this->paddleStrength = 0.8f;
	}

	AABB getBoatAABB() {
		// Base boat dimensions (scaled base mesh)
		float baseHalfWidth = 1.0f * 0.5f;   // Half of the width (scaled by 1.0 in x)
		float baseHalfHeight = 0.5f * 0.5f;  // Half of the height (scaled by 0.5 in y)
		float baseHalfDepth = 4.0f * 0.5f;   // Half of the depth (scaled by 3.0 in z)

		// Rotate the dimensions (we need the largest possible extents after rotation)
		// Use the boat's angle to adjust the AABB
		float sinAngle = sin(this->angle * (3.14159265f / 180.0f));
		float cosAngle = cos(this->angle * (3.14159265f / 180.0f));

		// Rotate the width and depth to find the new extents
		float rotatedHalfWidth = abs(baseHalfWidth * cosAngle) + abs(baseHalfDepth * sinAngle);
		float rotatedHalfDepth = abs(baseHalfWidth * sinAngle) + abs(baseHalfDepth * cosAngle);

		AABB box;

		// Add boat position and rotated extents
		box.min[0] = this->pos[0] - rotatedHalfWidth;
		box.min[1] = this->pos[1] - baseHalfHeight;  // Height remains the same (no rotation around y-axis)
		box.min[2] = this->pos[2] - rotatedHalfDepth;

		box.max[0] = this->pos[0] + rotatedHalfWidth;
		box.max[1] = this->pos[1] + baseHalfHeight;
		box.max[2] = this->pos[2] + rotatedHalfDepth;

		return box;
	}
};

