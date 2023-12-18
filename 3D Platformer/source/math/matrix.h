#pragma once
#include <cmath>

struct Mat4x4
{
	float data[16] = { 0.0f };
	Mat4x4() {}
	Mat4x4(const float* data)
	{
		for (int i{ 0 }; i < 16; i++)
			this->data[i] = data[i];
	}
	Mat4x4& identityMatrix()
	{
		data[0] = 1.0f;
		data[5] = 1.0f;
		data[10] = 1.0f;
		data[15] = 1.0f;
		return *this;
	}
	Mat4x4& translationMatrix(float x, float y, float z)
	{
		identityMatrix();
		data[3] = x;
		data[7] = y;
		data[11] = z;
		return *this;
	}
	//fov: radians, near: world units, far: world units, aspectRatio: window width / window height
	Mat4x4& perspectiveMatrix(float fov, float nearZ, float farZ, float aspectRatio)
	{
		data[0] = 1.0f / std::tanf(fov / 2.0f);
		data[5] = 1.0f / std::tanf(fov / 2.0f) * aspectRatio;
		data[10] = -(-farZ - nearZ) / (farZ - nearZ);
		data[11] = -2 * farZ * nearZ / (farZ - nearZ);
		data[14] = 1;
		return *this;
	}
	Mat4x4 euclidianRotationMatrix(float yawRadians, float pitchRadians)
	{
		float yc{ std::cosf(yawRadians) };
		float ys{ std::sinf(yawRadians) };
		float pc{ std::cosf(pitchRadians) };
		float ps{ std::sinf(pitchRadians) };
		float yawRotation[16] = {
			yc  ,0.0f, -ys,0.0f,
			0.0f,1.0f,0.0f,0.0f,
			ys  ,0.0f,  yc,0.0f,
			0.0f,0.0f,0.0f,1.0f,
		};
		float pitchRotation[16] = {
			1.0f,0.0f,0.0f,0.0f,
			0.0f,pc  , ps ,0.0f,
			0.0f,-ps  , pc ,0.0f,
			0.0f,0.0f,0.0f,1.0f,
		};
		return (Mat4x4)pitchRotation * (Mat4x4)yawRotation;
	}
	Mat4x4 operator*(Mat4x4 other);
	float& operator[](int index) { return data[index]; }
};

Mat4x4 Mat4x4::operator*(Mat4x4 other)
{
	Mat4x4 result;
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			for (int i = 0; i < 4; i++)
				result[x + y * 4] += data[i + y * 4] * other[x + i * 4];
	return result;
}
