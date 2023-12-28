#include "vector.h"

Vector3 cancelOppositeComponent(Vector3 direction, Vector3 vector)
{
	if (direction.dot(vector) < 0)
		return vector - direction * direction.dot(vector);
	else
		return vector;
}