#pragma once
#include "math/vector.h"
#include "math/matrix.h"

struct Camera
{
	Vector3 position;
	float pitch{ 0.0f };
	float yaw{ 0.0f };
	Mat4x4 getCameraMatrix()
	{
		return Mat4x4().euclidianRotationMatrix(yaw, pitch) * Mat4x4().translationMatrix(-position.x, -position.y, -position.z);
	}
};
