#pragma once
#include <array>
#include <cmath>
#include <stdio.h>
#include "aabb.h"

class BoundingSphere
{
public:
    std::array<float, 3> center;
    float radius;

    // Default constructor
    BoundingSphere() : center({ 0.0f, 0.0f, 0.0f }), radius(0.0f) {}

    // Constructor with center and radius
    BoundingSphere(const std::array<float, 3>& center, float radius) : center(center), radius(radius) {}

    // Constructor with individual coordinates
    BoundingSphere(float centerX, float centerY, float centerZ, float radius) : center({ centerX, centerY, centerZ }), radius(radius) {}

    // Static method to check if a sphere is colliding with a box
    static bool isColliding(BoundingSphere& sphere, AABB& box) {
        // Find the point on the box closest to the sphere's center
        float closestX = std::fmax(box.min[0], std::fmin(sphere.center[0], box.max[0]));
        float closestY = std::fmax(box.min[1], std::fmin(sphere.center[1], box.max[1]));
        float closestZ = std::fmax(box.min[2], std::fmin(sphere.center[2], box.max[2]));

        // Calculate the distance between the sphere's center and the closest point
        float distanceX = closestX - sphere.center[0];
        float distanceY = closestY - sphere.center[1];
        float distanceZ = closestZ - sphere.center[2];

        // Calculate the square of the distance
        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY) + (distanceZ * distanceZ);

        // A collision occurs if the distance is less than the sphere's radius
        return distanceSquared <= (sphere.radius * sphere.radius);
    }

    static void printBoundingSphere(BoundingSphere& sphere) {
        printf("center: %f %f %f | radius: %f\n", sphere.center[0], sphere.center[1], sphere.center[2], sphere.radius);
    }
};
