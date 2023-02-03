#pragma once
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>

struct Shader
{
	GLuint glID;
	char* source;

	Shader(GLuint shader_type, char* source = nullptr)
		:source(source)
	{
		//glID = 0;
		glID = glCreateShader(shader_type);
	}

	~Shader()
	{
		if (source)
			delete[] source;
		glDeleteShader(glID);
	}

	bool Compile()
	{
		glShaderSource(glID, 1, &source, NULL);
		glCompileShader(glID);
		int success;
		glGetShaderiv(glID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetShaderInfoLog(glID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
			return false;
		}
		return true;
	}

	bool AttachShader(GLuint program)
	{
		glAttachShader(program, glID);
		glLinkProgram(program);
		int status;
		glGetProgramiv(glID, GL_LINK_STATUS, &status);
		if (!status)
		{
			char infoLog[512];
			glGetProgramInfoLog(glID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::ATTACH_FAILED\n" << infoLog << std::endl;
			return false;
		}
		return true;
	}

	void SetSource(const char* src, bool compile = false)
	{
		if (src && source)
			delete[] source;

		if (src)
		{
			//set source to src
			source = new char[strlen(src) + 1];
			strcpy(source, src);
		}

		//print source
		printf("------Shader source:------\n");
		std::cout << source << std::endl;

		if (compile)
			Compile();
	}

	void SetSourceFromFile(const char* filePath, bool compile = false)
	{
		std::string content;
		std::ifstream fileStream(filePath, std::ios::in);

		if (!fileStream.is_open()) {
			std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
			return;
		}

		std::string line = "";
		while (!fileStream.eof()) {
			std::getline(fileStream, line);
			content.append(line + "\n");
		}

		fileStream.close();
		SetSource(content.c_str(), compile);
	}


};

class OpenGLProgram
{
public:
	OpenGLProgram()
	{
		glID = glCreateProgram();
		glEnable(GL_DEPTH_TEST);
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		
		vertexShader = new Shader(GL_VERTEX_SHADER);
		fragmentShader = new Shader(GL_FRAGMENT_SHADER);
	};
	~OpenGLProgram()
	{
		glDeleteProgram(glID);
		delete vertexShader;
		delete fragmentShader;
	};

	//void CreateProgram();
	void Use();
	//void DeleteProgram();
	
	/*
	* Read compile and attach shaders to the opengl program.
	*/
	bool CreatePipelineFromFiles(const char* filePathVert, const char* filePathFrag)
	{
		SetVertexShaderSourceFromFile(filePathVert);
		SetFragmentShaderSourceFromFile(filePathFrag);
		
		if (!CompileShaders())
			return false;
		
		AttachVertexShader();
		AttachFragmentShader();
	}
	/*
	* Assuming shader sources are already set this function will
	compile and attach shaders to the opengl program.
	*/
	bool CreatePipeline()
	{
		if (!CompileShaders())
			return false;

		AttachVertexShader();
		AttachFragmentShader();
	}
	

	bool AttachVertexShader();
	bool AttachFragmentShader();

	void SetVertexShaderSource(const char* src, bool compile = false);
	void SetFragmentShaderSource(const char* src, bool compile = false);

	void SetVertexShaderSourceFromFile(const char* filePath, bool compile = false);
	void SetFragmentShaderSourceFromFile(const char* filePath, bool compile = false);

	void SetVertexShader(Shader* shader);
	void SetFragmentShader(Shader* shader);
	
	bool CompileShaders();

	inline void SetUniform(const char* name, int value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniform1i(location, value);
	}
	inline void SetUniform(const char* name, float value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniform1f(location, value);
	}
	
	inline void SetUniform(const char* name, glm::vec2 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniform2f(location, value.x, value.y);
	}
	inline void SetUniform(const char* name, glm::vec3 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniform3f(location, value.x, value.y, value.z);
	}
	inline void SetUniform(const char* name, glm::vec4 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}

		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	inline void SetUniform(const char* name, glm::mat2 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]);
	}

	inline void SetUniform(const char* name, glm::mat3 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]);
	}
	
	inline void SetUniform(const char* name, glm::mat4 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
	}

	inline void SetGLClearFlags(GLbitfield flags)
	{
		clearFlags = flags;
	}
	inline void SetClearColor(glm::vec4 color)
	{
		clearColor = color;
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	}

	inline void Clear()
	{
		glClear(clearFlags);
	}
	
	GLuint glID;
private:
	Shader* vertexShader;
	Shader* fragmentShader;
	GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;
	glm::vec4 clearColor = glm::vec4(0.02f, 0.02f, 0.02f, 1.f);
};
