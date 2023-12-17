#pragma once

#include <vector>
#include <string>
#include "GL/gl3w.h"
#include "math/vector.h"
#include "shader.h"
#include "assimp/scene.h"

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

struct Texture
{
	unsigned int id = 0;
	std::string type;
	std::string path;
};


class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void draw(Shader shader);
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

class Model
{
public:
	Model(const char* path);
	void draw(Shader& shader);
private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> texturesLoaded;

	void loadModel(std::string path);
	Mesh proccessMesh(aiMesh* mesh, const aiScene* scene);
	void processNode(aiNode* node, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

};
