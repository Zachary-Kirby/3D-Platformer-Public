#pragma once

#include <vector>
#include "model.h"
#include "math/vector.h"
//the basic implementation idea is that each collider has to implement its own methods to handle other colliders

struct ColliderVertex
{
	Vector3 position;
	Vector3 normal;
};

struct CollisionPoint
{
	Vector3 position;
	float distance;
};

class MeshCollider
{
public:
	MeshCollider(const Model& modelRefrence);
	bool isCollidingWithSphere(Vector3 position, float radius);
	CollisionPoint collideSphere(Vector3 position, float radius);
private:
	std::vector<ColliderVertex> vertices;
	std::vector<unsigned int> indices;
};