#pragma once

#include <cmath>

struct Vector3
{
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
	Vector3 operator+(const Vector3 other) { return Vector3{ x + other.x, y + other.y, z + other.z }; }
	Vector3 operator-(const Vector3 other) { return Vector3{ x - other.x, y - other.y, z - other.z }; }
	Vector3 operator*(const float scalar) { return Vector3{ x * scalar,y * scalar,z * scalar }; }
	friend Vector3 operator*(const float& scalar, const Vector3& vector) { return Vector3{ vector.x * scalar,vector.y * scalar,vector.z * scalar }; }
	Vector3 operator/(const float scalar) { return Vector3{x/scalar, y/scalar, z/scalar}; }
	void operator+=(const Vector3 other) { x += other.x; y += other.y; z += other.z; }
	void operator-=(const Vector3 other) { x -= other.x; y -= other.y; z -= other.z; }
	float dot(const Vector3 other) { return x * other.x + y * other.y + z * other.z; }
	Vector3 cross(const Vector3 other) { return Vector3{ y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x }; }
	Vector3 operator-() { return Vector3{-x, -y, -z}; }
	float length() { return std::sqrtf(x * x + y * y + z * z); }
	Vector3 normalized() { 
		float l = length();
		return Vector3{ x / l, y / l, z / l };
	}
};


struct Vector2
{
	float x{ 0.0f };
	float y{ 0.0f };
	Vector2 operator+(const Vector2 other) { return Vector2{ x + other.x, y + other.y }; }
	Vector2 operator-(const Vector2 other) { return Vector2{ x - other.x, y - other.y }; }
	Vector2 operator*(const float scalar) { return Vector2{ x * scalar,y * scalar }; }
	friend Vector2 operator*(const float& scalar, const Vector2& vector) { return Vector2{ vector.x * scalar,vector.y * scalar }; }
	void operator+=(const Vector2 other) { x += other.x; y += other.y; }
	void operator-=(const Vector2 other) { x -= other.x; y -= other.y; }
	float dot(const Vector2 other) { return x * other.x + y * other.y; }
	Vector3 cross(const Vector2 other) { return Vector3{ 0, 0, x * other.y - y * other.x }; }
	Vector2 operator-() { return Vector2{ -x, -y }; }
	float length() { return std::sqrtf(x * x + y * y); }
	Vector2 normalized() {
		float l = length();
		return { x / l, y / l};
	}
	//if vector is 1,0 it will point 0,1
	Vector2 normalUp() { return { -y, x }; }
	//if vector is 1,0 it will point 0,-1
	Vector2 normalDown() { return { y, -x }; }
};


