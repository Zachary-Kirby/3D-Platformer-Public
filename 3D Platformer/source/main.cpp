#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include "SDL.h"
#include "SDL_image.h"
#include "GL/gl3w.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "colliders.h"

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

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	Window window(1600, 900);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
	//these should all be put into an init function or something
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
	
	Model room("data/models/basic_room.gltf");
	Model sphere("data/models/sphere.gltf");
	Model floor("data/models/test_floor.gltf");
	MeshCollider floorCollider(floor);

	Camera camera;
	camera.position = { 0.0f, 1.0f, -2.0f };
	
	float verts[] = {
		-0.5f,-0.5f, 0.0f,
		 0.5f,-0.5f, 0.0f,
		 0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0};
	
	Vector3 sphereLocation{ 0.0f, 2.0f, 0.0f };
	float sphereRadius{ 0.2f };

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
		
		if (keys[SDL_SCANCODE_J]) {
			sphereLocation.x += -std::cos(camera.yaw) * 0.02f;
			sphereLocation.z += std::sin(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_L]) {
			sphereLocation.x += std::cos(camera.yaw) * 0.02f;
			sphereLocation.z += -std::sin(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_I]) {
			sphereLocation.x += std::sin(camera.yaw) * 0.02f;
			sphereLocation.z += std::cos(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_K]) {
			sphereLocation.x += -std::sin(camera.yaw) * 0.02f;
			sphereLocation.z += -std::cos(camera.yaw) * 0.02f;
		}
		if (keys[SDL_SCANCODE_U]) {
			sphereLocation.y -= 0.02;
		}
		if (keys[SDL_SCANCODE_O]) {
			sphereLocation.y += 0.02;
		}

		if (keys[SDL_SCANCODE_SPACE])
			camera.position.y += 0.02f;
		if (keys[SDL_SCANCODE_LSHIFT])
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
				if (keys[SDL_SCANCODE_LCTRL])
				{
					sphereLocation.x += std::cos(camera.yaw) * 0.002f * sdlEvent.motion.xrel;
					sphereLocation.z += -std::sin(camera.yaw) * 0.002f * sdlEvent.motion.xrel;
					sphereLocation.y += -0.002f * sdlEvent.motion.yrel;
				}
				else
				{
					camera.yaw += sdlEvent.motion.xrel / (2.0f * 3.14f * 100.0f);
					camera.pitch += sdlEvent.motion.yrel / (2.0f * 3.14f * 100.0f);
				}
			}
			if (sdlEvent.type == SDL_MOUSEWHEEL)
			{
				if (keys[SDL_SCANCODE_LCTRL])
				{
					sphereLocation.x += std::sin(camera.yaw) * 0.08f * sdlEvent.wheel.y;
					sphereLocation.z += std::cos(camera.yaw) * 0.08f * sdlEvent.wheel.y;
				}
			}
		}
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float fov{ 100.0f };
		Mat4x4 cameraViewMatrix = Mat4x4().perspectiveMatrix(3.14f / 180.0f * fov, 0.125f, 100.0f, ((float)window.w / (float)window.h)) * camera.getCameraMatrix();
		//room.draw(defaultShader);
		defaultShader.bind();
		
		defaultShader.setInt("turnGreen", floorCollider.collideSphere(sphereLocation, sphereRadius));
		defaultShader.setMat4fv("camera", (cameraViewMatrix * Mat4x4().translationMatrix(sphereLocation.x, sphereLocation.y, sphereLocation.z) * Mat4x4().scalingMatrix(sphereRadius)).data);
		sphere.draw(defaultShader);

		defaultShader.setMat4fv("camera", cameraViewMatrix.data);
		defaultShader.setInt("turnGreen", 0);
		floor.draw(defaultShader);


		SDL_GL_SwapWindow(window.window);
		auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(mil - std::chrono::steady_clock::now());
		std::this_thread::sleep_for(std::chrono::milliseconds(16) - frameTime);
	}
	
	return 0;
}