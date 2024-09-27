#pragma once
#include "water_creature.h"
#include <stdio.h>

constexpr auto CREATURE_COUNT = 6;
class WaterCreatureManager
{
	static float getRandomIncrement() {
		return (rand() % 3 + static_cast<float>(1)) / 100;
	}
public:
	WaterCreature creatures[CREATURE_COUNT];

	WaterCreatureManager() {}

	void moveCreatures() {
		for (int i = 0; i < CREATURE_COUNT; i++) {
			auto &creature = creatures[i];
			std::pair<float, float>* dir = creature.getRotateAxis();

			// Move in the right axis
			creature.pos[0] = creature.pos[0] + (creature.speed / 60 * creature.direction * dir->first);
			creature.pos[2] = creature.pos[2] + (creature.speed / 60 * creature.direction * dir->second);

			creature.speedUp(getRandomIncrement());
			creature.angleUp(1.0f);

			if (creature.pos[0] > 100 || creature.pos[0] < -100 ||
				creature.pos[2] > 100 || creature.pos[2] < -100) { // Out of bounds
				this->respawnCreature(i);
			}
		}
	}

	void respawnCreature(int creatureIdx) {
		creatures[creatureIdx] = WaterCreature();
	}
};

