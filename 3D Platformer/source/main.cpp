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

struct DebugMesh
{
	std::vector<Vector3> vertices;
	unsigned int vao, vbo;
	DebugMesh()
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		
	}
	~DebugMesh()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
	void updateData()
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
	}
	void drawLines()
	{
		updateData();
		glLineWidth(3.0f);
		glDrawArrays(GL_LINES , 0, vertices.size());
		glBindVertexArray(0);
	}
};

#define pi 3.1415926535897932384626433832795
static float radians(float degrees) { return degrees / 180.0f * pi; }

/*
Now that the prototype is in a stable state it is time to clean it up... OR IS IT???
*/

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
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
	
	//Model room("data/models/basic_room.gltf");
	Model room("data/models/test_level.gltf");
	MeshCollider roomCollider(room);
	Model sphere("data/models/sphere.gltf");
	Model floor("data/models/test_floor.gltf");
	MeshCollider floorCollider(floor);

	Camera camera;
	camera.position = { 0.0f, 2.0f, -2.0f };
	
	float verts[] = {
		-0.5f,-0.5f, 0.0f,
		 0.5f,-0.5f, 0.0f,
		 0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0};
	
	
	DebugMesh debugLines;
	
	Vector3 sphereLocation{ 0.0f, 1.0f, 0.0f };
	float sphereRadius{ 0.5f };

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

	Vector3 playerVelocity;
	Vector3 cameraVelocity;
	Vector3 cameraFloorNormal{ 0.0f, 2.0f, 0.0f };
	const float cameraSpeed = 0.02f;
	const float cameraMaxFloorAngle = 60.0f;
	const float MAXSPEED = 0.05f*2;
	bool cameraGrounded = false;

	SDL_GameController* controller = NULL;
	std::cout << SDL_NumJoysticks() << "\n";
	bool exit{ false };
	while (!exit)
	{
		controller = SDL_GameControllerOpen(0);
		const unsigned char* keys = SDL_GetKeyboardState(NULL);
		
		{
			float moveX{ 0.0f };
			float moveZ{ 0.0f };
			Vector2 oldVelocity = Vector2{ cameraVelocity.x, cameraVelocity.z };
			if (controller)
			{
				float controllerLX = (- (int)(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f * 7.0f)) / 7.0f;
				float controllerLY = (- (int)(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f * 7.0f)) / 7.0f;
				moveX += -std::cos(camera.yaw) * controllerLX * cameraSpeed;
				moveZ += std::sin(camera.yaw) * controllerLX * cameraSpeed;
				moveX += std::sin(camera.yaw) * controllerLY * cameraSpeed;
				moveZ += std::cos(camera.yaw) * controllerLY * cameraSpeed;

				float controllerRX = (-(int)(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f * 7.0f)) / 7.0f;
				float controllerRY = (-(int)(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f * 7.0f)) / 7.0f;
				camera.yaw += -controllerRX*std::abs(controllerRX) / 180.0f * 3.14 * 8.0f;
				camera.pitch += -controllerRY*std::abs(controllerRY) / 180.0f * 3.14 * 8.0f;
			}

			if (keys[SDL_SCANCODE_A]) {
				moveX += -std::cos(camera.yaw) * cameraSpeed;
				moveZ += std::sin(camera.yaw) * cameraSpeed;
				
			}
			if (keys[SDL_SCANCODE_D]) {
				moveX += std::cos(camera.yaw) * cameraSpeed;
				moveZ += -std::sin(camera.yaw) * cameraSpeed;
				
			}
			if (keys[SDL_SCANCODE_W]) {
				moveX += std::sin(camera.yaw) * cameraSpeed;
				moveZ += std::cos(camera.yaw) * cameraSpeed;
				
				
			}
			if (keys[SDL_SCANCODE_S]) {
				moveX += -std::sin(camera.yaw) * cameraSpeed;
				moveZ += -std::cos(camera.yaw) * cameraSpeed;
				
			}
			float moveY = (-moveX * cameraFloorNormal.x - moveZ * cameraFloorNormal.z) / cameraFloorNormal.y;
			cameraVelocity.x += moveX;
			cameraVelocity.y += moveY;
			cameraVelocity.z += moveZ;
			Vector2 newVelocity = Vector2{cameraVelocity.x, cameraVelocity.z};
			
			if (oldVelocity.length() < newVelocity.length() && newVelocity.length() > MAXSPEED)
			{
				if (oldVelocity.length() > MAXSPEED)
					newVelocity = newVelocity.normalized() * oldVelocity.length();
				else
					newVelocity = newVelocity.normalized() * MAXSPEED;
				cameraVelocity.x = newVelocity.x;
				cameraVelocity.z = newVelocity.y;
			}
		}
		
		

		if (keys[SDL_SCANCODE_J]) {
			playerVelocity.x += -std::cos(camera.yaw) * cameraSpeed;
			playerVelocity.z += std::sin(camera.yaw) * cameraSpeed;
		}
		if (keys[SDL_SCANCODE_L]) {
			playerVelocity.x += std::cos(camera.yaw) * cameraSpeed;
			playerVelocity.z += -std::sin(camera.yaw) * cameraSpeed;
		}
		if (keys[SDL_SCANCODE_I]) {
			playerVelocity.x += std::sin(camera.yaw) * cameraSpeed;
			playerVelocity.z += std::cos(camera.yaw) * cameraSpeed;
		}
		if (keys[SDL_SCANCODE_K]) {
			playerVelocity.x += -std::sin(camera.yaw) * cameraSpeed;
			playerVelocity.z += -std::cos(camera.yaw) * cameraSpeed;
		}
		if (keys[SDL_SCANCODE_U]) {
			playerVelocity.y -= 0.02;
		}
		if (keys[SDL_SCANCODE_O]) {
			playerVelocity.y += 0.02;
		}
		if (keys[SDL_SCANCODE_E])
		{
			cameraVelocity.y = 0.0f;
			camera.position.y += 0.1f;
		}
		if (keys[SDL_SCANCODE_Q])
		{
			cameraVelocity.y = 0.0f;
			camera.position.y -= 0.1f;
		}
		if (keys[SDL_SCANCODE_LSHIFT])
		{
			cameraVelocity.y = 0.005f;
		}


		if (keys[SDL_SCANCODE_SPACE] && cameraGrounded)
			cameraVelocity.y += 0.15f;
		if (controller)
		{
			if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A) && cameraGrounded)
				cameraVelocity.y += 0.1f;
		}
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

		playerVelocity.y -= 0.005f;
		sphereLocation += playerVelocity;

		CollisionPoint collisionPoint = roomCollider.collideSphere(sphereLocation, sphereRadius);
		{
			float distance = (sphereLocation - collisionPoint.position).length();
			if (distance < sphereRadius)
			{
				Vector3 pushDirection = (sphereLocation - collisionPoint.position) / distance;
				sphereLocation -= pushDirection * (distance - sphereRadius );
				if (pushDirection.dot(playerVelocity) < 0)
					playerVelocity -= pushDirection.dot(playerVelocity) * pushDirection;

			}
		}

		bool playerFriction = true;
		cameraGrounded = false;
		cameraVelocity.y -= 0.005f;
		camera.position += cameraVelocity;
		cameraFloorNormal = { 0.0f,1.0f,0.0f };
		for (int i{0}; i < 4; i++)
		{
			collisionPoint = roomCollider.collideSphere(camera.position, sphereRadius);
			{
				float distance = (camera.position - collisionPoint.position).length();
				if (distance <=  sphereRadius)
				{
				
					Vector3 pushDirection = (camera.position - collisionPoint.position) / distance;
					float pushAngle = std::acosf(pushDirection.dot(collisionPoint.normal));
					bool onEdge = pushAngle >= radians(0.1f) && pushAngle <= radians(179.0f);
					//note the I can tell if the point lies on the boundary by doing a check between the normal of the triangle and the push direction. If they don't point the same directon it's on the edge
					if (pushDirection.dot(cameraVelocity) < 0)
					{
					
						if (!onEdge && std::acosf(pushDirection.y) <= radians(cameraMaxFloorAngle))
						{
							CollisionPoint playerSnapPoint{ rayPlaneIntersection(camera.position + collisionPoint.normal * -sphereRadius, Vector3{ 0.0f, 1.0f, 0.0f }, collisionPoint.position, collisionPoint.normal) };
							if (!playerSnapPoint.failed)
							{
								camera.position.y += playerSnapPoint.distance;
								cameraGrounded = true;
								cameraFloorNormal = collisionPoint.normal;
								cameraVelocity.y -= (collisionPoint.normal.dot(cameraVelocity) * collisionPoint.normal).y; // this line of code is essential for making slopes smooth to traverse
							}
							else
							{
								std::cout << "ERROR::FLOOR_COLLISION\nFAILED TO FIND A FLOOR\n";
							}
						}
						else
						{
							//only make it slippery off the edge not along the edge
							//TODO move the slowdown code outside the loop so it only happens once a frame
							if (onEdge)
							{
								if (pushDirection.y > 0)
								{
									//Vector3 edgeNormal = Vector3{ pushDirection.x, 0, pushDirection.z }.normalized();
									Vector3 edgeDirection = Vector3{-pushDirection.z, 0, pushDirection.x}.normalized();
									//cameraVelocity -= edgeDirection * edgeDirection.dot(cameraVelocity) * 0.2f;
									//if (edgeNormal.dot(cameraVelocity) < 0)
									//	cameraVelocity -= edgeNormal * edgeNormal.dot(cameraVelocity) * 0.8f;
									playerFriction = false;
								}
							}
							cameraVelocity -= pushDirection.dot(cameraVelocity) * pushDirection;
							camera.position -= pushDirection * (distance - sphereRadius);
						}
					}
					


				}
			}
		
		}
		if (playerFriction)
		{
			cameraVelocity.x *= 0.8f;
			cameraVelocity.z *= 0.8f;
		}
		cameraVelocity.y = std::clamp(cameraVelocity.y, -1.0f, 1.0f);
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float fov{ 100.0f };
		Mat4x4 cameraViewMatrix = Mat4x4().perspectiveMatrix(3.14f / 180.0f * fov, 0.125f, 100.0f, ((float)window.w / (float)window.h)) * camera.getCameraMatrix();
		defaultShader.bind();
		
		defaultShader.setInt("turnGreen", 0);
		defaultShader.setMat4fv("camera", cameraViewMatrix.data);
		room.draw(defaultShader);
		
		defaultShader.setInt("turnGreen", 0);
		defaultShader.setMat4fv("camera", (cameraViewMatrix * Mat4x4().translationMatrix(sphereLocation.x, sphereLocation.y, sphereLocation.z) * Mat4x4().scalingMatrix(sphereRadius)).data);
		sphere.draw(defaultShader);

		

		debugLines.vertices.push_back(camera.position + Vector3{ std::sinf(camera.yaw)*0.2f, -sphereRadius + 0.1f, std::cosf(camera.yaw) * 0.2f });
		debugLines.vertices.push_back(camera.position + cameraVelocity + Vector3{ std::sinf(camera.yaw) * 0.2f, -sphereRadius+0.1f, std::cosf(camera.yaw) * 0.2f });
		basicShader.bind();
		basicShader.setMat4fv("camera", cameraViewMatrix.data);
		debugLines.drawLines();
		debugLines.vertices.clear();

		//DRAW THE DEBUG FLOOR
		//defaultShader.setMat4fv("camera", cameraViewMatrix.data);
		//defaultShader.setInt("turnGreen", 0);
		//floor.draw(defaultShader);

		//DEBUG THE COLLISION POINT
		glDisable(GL_DEPTH_TEST);
		defaultShader.bind();
		defaultShader.setInt("turnGreen", 1);
		defaultShader.setMat4fv("camera", (cameraViewMatrix* Mat4x4().translationMatrix(collisionPoint.position.x, collisionPoint.position.y, collisionPoint.position.z)* Mat4x4().scalingMatrix(0.05f)).data);
		sphere.draw(defaultShader);
		glEnable(GL_DEPTH_TEST);


		SDL_GL_SwapWindow(window.window);
		auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(mil - std::chrono::steady_clock::now());
		std::this_thread::sleep_for(std::chrono::milliseconds(16) - frameTime);
	}
	
	return 0;
}