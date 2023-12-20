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
bool MeshCollider::collideSphere(Vector3 position, float radius)
{
	//loop through every triangle face in the mesh and check if the distance between the sphere and the surface is less than the radius
	//and check if the sphere's center point when casted to the triangle surface is within radius of the triangle in its plane
	
	for (unsigned int i{ 0 }; i < indices.size(); i += 3)
	{
		Vector3& point1 = vertices[indices[i]].position;
		Vector3& point2 = vertices[indices[i+1]].position;
		Vector3& point3 = vertices[indices[i+2]].position;
		Vector3& normal = vertices[i].normal;
		float distanceFromSurface = normal.dot(position - point1);
		if (abs(distanceFromSurface) > radius)
			continue;
		Vector3 boundary = (normal).cross(point2 - point1).normalized();
		if (boundary.dot(position - point1) > radius)
			continue;
		boundary = (normal).cross(point3 - point2).normalized();
		if (boundary.dot(position - point2) > radius)
			continue;
		boundary = (normal).cross(point1 - point3).normalized();
		if (boundary.dot(position - point3) > radius)
			continue;

		return true;
	}
	return false;
}
