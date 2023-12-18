#pragma once

class Shader
{
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
	void bind();
	void setInt(const char* name, int data);
	unsigned int programId;
};
