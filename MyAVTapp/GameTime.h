#pragma once
#include <string>
#include <iomanip> // For std::setw and std::setfill
#include <sstream> // For std::ostringstream
class GameTime
{
	int minutes = 0;
	int seconds = 0;
public:
	GameTime() {}
	GameTime(int elapsedSeconds) {
		this->minutes = elapsedSeconds / 60;
		this->seconds = elapsedSeconds % 60;
	}

	void reset() {
		this->minutes = 0;
		this->seconds = 0;
	}

	void addSecond() {
		this->seconds++;
		if (this->seconds >= 60) {
			this->minutes++;
			this->seconds = 0;
		}
	}

	std::string formatTime() const {
		std::ostringstream oss;
		oss << std::setw(2) << std::setfill('0') << this->minutes
			<< ":"
			<< std::setw(2) << std::setfill('0') << this->seconds;
		return oss.str();
	}
};

