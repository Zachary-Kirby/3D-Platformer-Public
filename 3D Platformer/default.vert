#version 460 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
uniform mat4x4 camera;

out vec2 texCoord;
out vec3 fragNormal;

void main()
{
	texCoord = uv;
	fragNormal = normal;
	gl_Position = camera * vec4(pos.x, pos.y, pos.z, 1.0);
}