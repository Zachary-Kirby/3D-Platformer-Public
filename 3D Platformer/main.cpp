#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include "SDL.h"
#include "SDL_image.h"
#include "GL/gl3w.h"
#include "fileloader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

class Window
{
public:
	Window(int w, int h)
	{
		this->w = w;
		this->h = h;
		window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		glContext = SDL_GL_CreateContext(window);
		if (!glContext)
		{
			std::cout << "failed to make a OpenGL context\n";
		}
	}
	~Window()
	{
		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	SDL_Window* window;
	SDL_GLContext glContext;
	int w;
	int h;
};

class Shader
{
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		unsigned int vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		char* vertexShaderFile = loadText(vertexShaderPath);
		glShaderSource(vertexShaderId, 1, &vertexShaderFile, NULL);
		delete[] vertexShaderFile;
		glCompileShader(vertexShaderId);
		{
			int success;
			char infoLog[512];
			glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
				std::cout << "SHADER::VERTEX::FAILED\n" << infoLog;
			}
		}
		unsigned int fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
		char* fragmentShaderFile = loadText(fragmentShaderPath);
		glShaderSource(fragmentShaderId, 1, &fragmentShaderFile, NULL);
		delete[] fragmentShaderFile;
		glCompileShader(fragmentShaderId);
		{
			int success;
			char infoLog[512];
			glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragmentShaderId, 512, NULL, infoLog);
				std::cout << "SHADER::FRAGMENT::FAILED\n" << infoLog;
			}
		}
		programId = glCreateProgram();
		glAttachShader(programId, vertexShaderId);
		glAttachShader(programId, fragmentShaderId);
		glLinkProgram(programId);
		{
			int success;
			char infoLog[512];
			glGetProgramiv(programId, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(programId, 512, NULL, infoLog);
				std::cout << "SHDARE::LINK::FAILED\n" << infoLog;
			}
		}
		glDeleteShader(vertexShaderId);
		glDeleteShader(fragmentShaderId);
	}
	void bind() { glUseProgram(programId); }
	void setInt(const char* name, int data)
	{
		unsigned int uniformLocation = glGetUniformLocation(programId, name);
		glUniform1iv(uniformLocation, 1, &data);
	}
	unsigned int programId;
};

struct Vector3
{
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };
};

struct Vector2
{
	float x{ 0.0f };
	float y{ 0.0f };
};

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

unsigned int loadTextureFromFile(std::string fileName, std::string directory)
{
	SDL_Surface* surface = IMG_Load((directory + "/" + fileName).c_str());
	unsigned int textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(surface);
	return textureId;
}

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	{
		this->vertices = vertices;
		this->indices  = indices;
		this->textures = textures;
		this->setupMesh();
	}
	void draw(Shader shader)
	{
		shader.bind();
		unsigned int diffuseNumber{ 1 };
		unsigned int specularNumber{ 1 };
		for (unsigned int i{0}; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNumber++);
			if (name == "texture_specular")
				number = std::to_string(specularNumber++);
			shader.setInt(("material." + name + number).c_str(), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
	}
};

class Model
{
public:
	Model(const char* path)
	{
		loadModel(path);
	}
	void draw(Shader &shader);
private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> texturesLoaded;

	void loadModel(std::string path);
	Mesh proccessMesh(aiMesh* mesh, const aiScene* scene);
	void processNode(aiNode* node, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	
};

void Model::draw(Shader& shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].draw(shader);
}

void Model::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip{ false };
		//check if time can be saved by just refrencing one of the already loaded textures
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			if (std::strcmp(str.C_Str(), texturesLoaded[i].path.c_str()) == 0)
			{
				textures.push_back(texturesLoaded[i]);
				skip = true;
				break;
			}
		}
		//if texture was already loaded don't create another. Else, create a texture and add it
		//to the already loaded list.
		if (!skip)
		{
			Texture texture;
			texture.id = loadTextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			texturesLoaded.push_back(texture);
		}
	}
	return textures;
}

Mesh Model::proccessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		if (mesh->mTextureCoords[0])
		{
			vertex.uv.x = mesh->mTextureCoords[0][i].x;
			vertex.uv.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.uv = Vector2{ 0, 0 };
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int ii = 0; ii < face.mNumIndices; ii++)
			indices.push_back(face.mIndices[ii]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

void Model::processNode(aiNode * node, const aiScene * scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(proccessMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

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
		data[3]  = x;
		data[7]  = y;
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

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	Window window(1600, 900);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	int errorCode = gl3wInit();
	if (errorCode)
	{
		std::cout << "gl3w failed to Init. Error code: " << errorCode;
		return 0;
	}
	glViewport(0, 0, window.w, window.h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	Shader defaultShader("default.vert", "default.frag");
	Shader basicShader("basic.vert", "basic.frag");
	
	Model house("data/models/house.gltf");

	Camera camera;
	
	float verts[] = {
		-0.5f,-0.5f, 0.0f, 
		 0.5f,-0.5f, 0.0f,
		 0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0};
	

	unsigned int vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	
	bool exit{ false };
	while (!exit)
	{
		
		const unsigned char* keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_A]) {
			camera.position.x += -std::cos(camera.yaw) * 0.02f;
			camera.position.z +=  std::sin(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_D]) {
			camera.position.x +=  std::cos(camera.yaw) * 0.02f;
			camera.position.z += -std::sin(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_W]) {
			camera.position.x += std::sin(camera.yaw) * 0.02f;
			camera.position.z += std::cos(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_S]) {
			camera.position.x += -std::sin(camera.yaw) * 0.02f;
			camera.position.z += -std::cos(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_SPACE])
			camera.position.y += 0.02f;
		if (keys[SDL_SCANCODE_LCTRL])
			camera.position.y -= 0.02f;
		if (keys[SDL_SCANCODE_ESCAPE])
			exit = true;
		

		auto mil = std::chrono::steady_clock::now();
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT)
				exit = true;
			if (sdlEvent.type == SDL_MOUSEMOTION)
			{
				camera.yaw += sdlEvent.motion.xrel / (2.0f * 3.14f * 100.0f);
				camera.pitch += sdlEvent.motion.yrel / (2.0f * 3.14f * 100.0f);
			}
		}
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		basicShader.bind();
		unsigned int cameraUniformLocation = glGetUniformLocation(basicShader.programId, "camera");
		glProgramUniformMatrix4fv(basicShader.programId, cameraUniformLocation, 1, GL_TRUE, (Mat4x4().perspectiveMatrix(3.14f /180.0f*70.0f, 0.125f, 100.0f, ((float)window.w / (float)window.h)) * camera.getCameraMatrix()).data);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
		cameraUniformLocation = glGetUniformLocation(defaultShader.programId, "camera");
		glProgramUniformMatrix4fv(defaultShader.programId, cameraUniformLocation, 1, GL_TRUE, (Mat4x4().perspectiveMatrix(3.14f / 180.0f * 70.0f, 0.125f, 100.0f, ((float)window.w / (float)window.h)) * camera.getCameraMatrix()).data);
		
		house.draw(defaultShader);


		SDL_GL_SwapWindow(window.window);
		auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(mil - std::chrono::steady_clock::now());
		std::this_thread::sleep_for(std::chrono::milliseconds(16) - frameTime);
	}
	
	return 0;
}