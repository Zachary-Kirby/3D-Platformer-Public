#include "GL/gl3w.h"
#include "shader.h"
#include "fileloader.h"
#include <iostream>

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
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

void Shader::bind()
{
	glUseProgram(programId);
}

void Shader::setInt(const char* name, int data)
{
	unsigned int uniformLocation = glGetUniformLocation(programId, name);
	glUniform1iv(uniformLocation, 1, &data);
}

void Shader::setMat4fv(const char* name, float* data)
{
	unsigned int uniformLocation = glGetUniformLocation(programId, name);
	glUniformMatrix4fv(uniformLocation, 1, GL_TRUE, data);
}


//bools use setInt
