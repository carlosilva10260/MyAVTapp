#pragma once
#include <array>

class Camera
{
	std::array<float, 3> camPos;
	std::array<float, 3> camTarget;
	std::array<float, 3> camUp;

	int type;
	float alpha;
	float beta;
	float r; // dist to the boat

public:
	Camera(int type = 0) : type(type), camPos({ 0.0f, 0.0f, 0.0f }), camTarget({ 0.0f, 0.0f, 0.0f }), camUp({0.0f, 1.0f, 0.0f}), alpha(0.0f), beta(30.0f), r(10.0f) {}

	void setPos(const std::array<float, 3>& pos) {
		camPos = pos;
	}

	std::array<float, 3> getPos(void) const {
		return camPos;
	}

	void setTarget(const std::array<float, 3>& target) {
		camTarget = target;
	}

	std::array<float, 3> getTarget(void) const {
		return camTarget;
	}

	void setType(int type) {
		this->type = type;
	}

	int getType(void) const {
		return type;
	}

	void setUp(const std::array<float, 3>& up) {
		camUp = up;
	}

	std::array<float, 3> getUp(void) const {
		return camUp;
	}

	void setAlpha(float angle) { alpha = angle; }
	float getAlpha() const { return alpha; }

	void setBeta(float angle) { beta = angle; }
	float getBeta() const { return beta; }

	void setR(float radius) { r = radius; }
	float getR() const { return r; }
};