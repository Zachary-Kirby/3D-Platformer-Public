#pragma once

struct Vector3
{
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
	Vector3 operator+(const Vector3 other) { return Vector3{ x + other.x, y + other.y, z + other.z }; }
	Vector3 operator-(const Vector3 other) { return Vector3{ x - other.x, y - other.y, z - other.z }; }
	Vector3 operator*(const float scalar) { return Vector3{ x * scalar,y * scalar,z * scalar }; }
	float dot(const Vector3 other) { return x * other.x + y * other.y + z * other.z; }
	Vector3 cross(const Vector3 other) { return Vector3{ y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x }; }
	Vector3 operator-() { return Vector3{-x, -y, -z}; }
};

struct Vector2
{
	float x{ 0.0f };
	float y{ 0.0f };
	Vector2 operator+(const Vector2 other) { return Vector2{ x + other.x, y + other.y }; }
	Vector2 operator-(const Vector2 other) { return Vector2{ x - other.x, y - other.y }; }
	Vector2 operator*(const float scalar) { return Vector2{ x * scalar,y * scalar }; }
	float dot(const Vector2 other) { return x * other.x + y * other.y; }
	Vector3 cross(const Vector2 other) { return Vector3{ 0, 0, x * other.y - y * other.x }; }
	Vector2 operator-() { return Vector2{ -x, -y }; }
};
