#pragma once

class Shader
{
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
	void bind();
	void setInt(const char* name, int data);
	void setMat4fv(const char* name, float* data);
	unsigned int programId;
};
