#pragma once
#include <array>
#include <stdio.h>
class AABB // Axis Aligned Bounding Box
{
public:
	std::array<float, 3> min;
	std::array<float, 3> max;

    AABB() : min({ 0.0f, 0.0f, 0.0f }), max({ 0.0f, 0.0f, 0.0f }) {}

    AABB(std::array<float, 3> &min, std::array<float, 3> &max) : min(min), max(max) {}
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : 
        min({ minX, minY, minZ }), max({ maxX, maxY, maxZ }) {}

    static bool isColliding(AABB& box1, AABB& box2) {
        // Check for overlap along each axis
        bool overlapX = box1.max[0] >= box2.min[0] && box1.min[0] <= box2.max[0];
        bool overlapY = box1.max[1] >= box2.min[1] && box1.min[1] <= box2.max[1];
        bool overlapZ = box1.max[2] >= box2.min[2] && box1.min[2] <= box2.max[2];

        // Collision occurs if there is overlap along all axes
        return overlapX && overlapY && overlapZ;
    }

    static void printBoundingBox(AABB& box) {
        printf("min: %f %f %f | max: %f %f %f\n", box.min[0], box.min[1], box.min[2], box.max[0], box.max[1], box.max[2]);
    }
};

