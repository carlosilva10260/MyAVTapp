#pragma once
#include <array>

class Camera
{
	std::array<float, 3> camPos;
	std::array<float, 3> camTarget;
	std::array<float, 3> camUp;

	int type;

public:
	Camera(int type = 0) : type(type), camPos({ 0.0f, 0.0f, 0.0f }), camTarget({ 0.0f, 0.0f, 0.0f }), camUp({0.0f, 1.0f, 0.0f}) {}

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
};

