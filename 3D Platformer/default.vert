#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
uniform mat4x4 camera;

out vec2 texCoord;

void main()
{
	texCoord = uv;
	gl_Position = camera * vec4(pos.x, pos.y, pos.z, 1.0);
}