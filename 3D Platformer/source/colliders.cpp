#include "colliders.h"
#include <cmath>
#include <iostream>

MeshCollider::MeshCollider(const Model& modelRefrence)
{
	//TODO normals from the mesh may be unreliable so finding it myself is preferable
	//go through every mesh in the model and construct a more compact version with only using the positions and normals
	//TODO there is a bug where indices get mismatched because there are mutliple meshes and indices are relitive to inside their respective mesh...
	for (unsigned int meshId{0}; meshId < modelRefrence.meshes.size(); meshId++)
	{
		const Mesh& mesh = modelRefrence.meshes[meshId];
		for (unsigned int i{0}; i < mesh.indices.size(); i++)
		{
			indices.push_back(mesh.indices[i]);
		}
		for (unsigned int i{0}; i < mesh.vertices.size(); i++)
		{
			vertices.push_back({ mesh.vertices[i].position, Vector3() });
		}
		//TODO normal only needs to be stored once
		for (unsigned int i{0}; i < indices.size(); i += 3)
		{
			ColliderVertex& vertex1 = vertices[indices[i]];
			ColliderVertex& vertex2 = vertices[indices[i+1]];
			ColliderVertex& vertex3 = vertices[indices[i+2]];
			Vector3 normal = (vertex3.position - vertex2.position).cross(vertex2.position - vertex1.position).normalized();
			vertex1.normal = vertex2.normal = vertex3.normal = normal;
		}
	}
}
//TODO there is a possible optimization by precaclulating the outward normals of the edges of the triangle
//TODO there is a bug where the sphere can collide if it is touching 2 of the lines 'infinite extensions'
bool MeshCollider::isCollidingWithSphere(Vector3 position, float radius)
{
	//loop through every triangle face in the mesh and check if the distance between the sphere and the surface is less than the radius
	//and check if the sphere's center point when casted to the triangle surface is within radius of the triangle in its plane
	/*
	1. get the dist ance to to the surface
	2. get the point closest to the sphere on the plane
	3. clamp it to the triangle
	4. get the distance to the sphere
	*/
	for (unsigned int i{ 0 }; i < indices.size(); i += 3)
	{
		Vector3& point1 = vertices[indices[i]].position;
		Vector3& point2 = vertices[indices[i+1]].position;
		Vector3& point3 = vertices[indices[i+2]].position;
		Vector3& normal = vertices[indices[i]].normal;
		
		float distanceFromSurface = normal.dot(position - point1);
		if (abs(distanceFromSurface) > radius)
			continue;
		
		Vector3 boundaryNormal = (normal).cross(point2 - point1).normalized();
		float distanceFromBoundary = boundaryNormal.dot(position - point1);
		if (distanceFromBoundary > radius || distanceFromBoundary < boundaryNormal.dot(point3 - point1) - radius)
			continue;
		
		boundaryNormal = (normal).cross(point3 - point2).normalized();
		distanceFromBoundary = boundaryNormal.dot(position - point2);
		if (distanceFromBoundary > radius || distanceFromBoundary < boundaryNormal.dot(point1 - point2) - radius)
			continue;

		boundaryNormal = (normal).cross(point1 - point3).normalized();
		distanceFromBoundary = boundaryNormal.dot(position - point3);
		if (distanceFromBoundary > radius || distanceFromBoundary < boundaryNormal.dot(point2 - point3) - radius)
			continue;
		
		return true;
	}
	return false;
}

CollisionPoint MeshCollider::collideSphere(Vector3 position, float radius)
{
	float totalClosestDistance = -1;
	Vector3 totalClosestPoint;
	for (unsigned int i{ 0 }; i < indices.size(); i += 3)
	{
		Vector3& point1 = vertices[indices[i]].position;
		Vector3& point2 = vertices[indices[i + 1]].position;
		Vector3& point3 = vertices[indices[i + 2]].position;
		Vector3& normal = vertices[indices[i]].normal;

		float distanceFromSurface = normal.dot(position - point1);
		if (abs(distanceFromSurface) > radius)
			continue;

		//snap to a point over one of the sides
		Vector3 closestPoint = position - normal * distanceFromSurface; //starts out with a point on the plane that contains the triangle
		Vector3 line12Normal = (normal).cross(point2 - point1).normalized();//all can be precalculated
		Vector3 line23Normal = (normal).cross(point3 - point2).normalized();
		Vector3 line31Normal = (normal).cross(point1 - point3).normalized();
		
		
		float line12Distance = line12Normal.dot(closestPoint - point1);
		float farthestOppositeLine12 = line12Normal.dot(point3 - point1);
		if (line12Distance < farthestOppositeLine12)
			closestPoint += line12Normal * (farthestOppositeLine12 - line12Distance);

		float line23Distance = line23Normal.dot(closestPoint - point2);
		float farthestOppositeLine23 = line23Normal.dot(point1 - point2);
		if (line23Distance < farthestOppositeLine23)
			closestPoint += line23Normal * (farthestOppositeLine23 - line23Distance);

		float line31Distance = line31Normal.dot(closestPoint - point3);
		float farthestOppositeLine31 = line31Normal.dot(point2 - point3);
		if (line31Distance < farthestOppositeLine31)
			closestPoint += line31Normal * (farthestOppositeLine31 - line31Distance);
		
		
		line12Distance = line12Normal.dot(closestPoint - point1);
		line23Distance = line23Normal.dot(closestPoint - point2);
		line31Distance = line31Normal.dot(closestPoint - point3);

		//snap to one of the sides ( if not already in the triangle )
		if (line12Distance > 0)
			closestPoint -= line12Normal * line12Distance;
		if (line23Distance > 0)
			closestPoint -= line23Normal * line23Distance;
		if (line31Distance > 0)
			closestPoint -= line31Normal * line31Distance;

		//get the distance to the point and determine if it is the closest
		float closestDistance = (position - closestPoint).length();
		if (closestDistance < totalClosestDistance || totalClosestDistance < 0)
		{
			totalClosestPoint = closestPoint;
			totalClosestDistance = closestDistance;
		}
	}
	
	return { totalClosestPoint, totalClosestDistance};
}


/*
this is just a less efficient way of doing the same thing... This may be useful for other the final collision resolution though

Vector3& point1 = vertices[indices[i]].position;
Vector3& point2 = vertices[indices[i+1]].position;
Vector3& point3 = vertices[indices[i+2]].position;
Vector3& normal = vertices[i].normal;
Vector3 closestPoint = position;

float distanceFromSurface = normal.dot(position - point1);
if (abs(distanceFromSurface) > radius)
	continue;
closestPoint = closestPoint - normal * distanceFromSurface;

Vector3 boundaryNormal = (normal).cross(point2 - point1).normalized();
float distanceFromBoundary = boundaryNormal.dot(position - point1);
if (distanceFromBoundary > 0)
	closestPoint -= distanceFromBoundary * boundaryNormal;

boundaryNormal = (normal).cross(point3 - point2).normalized();
distanceFromBoundary = boundaryNormal.dot(position - point2);
if (distanceFromBoundary > 0)
	closestPoint -= distanceFromBoundary * boundaryNormal;

boundaryNormal = (normal).cross(point1 - point3).normalized();
distanceFromBoundary = boundaryNormal.dot(position - point3);
if (distanceFromBoundary > 0)
	closestPoint -= distanceFromBoundary * boundaryNormal;

if ((position - closestPoint).length() <= radius)
	return true;
*/