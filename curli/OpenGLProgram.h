#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <vector>

//define a macro that takes a function calls it and ctaches opengl errors
#define GL_CALL(func) \
do { \
    func; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        const char* errorString = (const char*)glad_glGetString(GL_VERSION); \
        const char* description = (const char*)glfwGetError(NULL); \
        printf("OpenGL error %d (%s) at %s:%d - %s\n", error, errorString, __FILE__, __LINE__, description); \
    } \
} while (false)

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
		GL_CALL(glDeleteShader(glID));
	}

	bool Compile()
	{
		GL_CALL(glShaderSource(glID, 1, &source, NULL));
		GL_CALL(glCompileShader(glID));
		int success;
		GL_CALL(glGetShaderiv(glID, GL_COMPILE_STATUS, &success));
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
		GL_CALL(glAttachShader(program, glID));
		GL_CALL(glLinkProgram(program));
		int status;
		GL_CALL(glGetProgramiv(glID, GL_LINK_STATUS, &status));
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

struct VertexBufferObject
{
public:
	GLuint glID = -1;
	unsigned int dataSize = 0;

	std::string attribName = "";
	GLint attribSize = 0; // 1, 2, 3, 4
	GLenum type; // GL_FLOAT, GL_DOUBLE, GL_INT, GL_UNSIGNED_INT, GL_SHORT, GL_UNSIGNED_SHORT, GL_BYTE, GL_UNSIGNED_BYTE
	GLuint stride = 0;
	GLuint offset = 0;
	GLboolean normalized = GL_FALSE;
	GLenum usage = GL_STATIC_DRAW;

	VertexBufferObject(void* data, unsigned int dataSize, GLenum type, std::string attribName, GLenum attribSize, GLuint programID,
		GLenum usage = GL_STATIC_DRAW, GLuint stride = 0, GLuint offset = 0, GLboolean normalized = GL_FALSE)
		:dataSize(dataSize), type(type), attribName(attribName), attribSize(attribSize), stride(stride), offset(offset)
	{
		GL_CALL(glGenBuffers(1, &glID));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, glID));

		unsigned int t_size = 0;
		switch (type)
		{
		case GL_FLOAT:
			data = (float*)data;
			t_size = sizeof(float);
			break;
		case GL_DOUBLE:
			data = (double*)data;
			t_size = sizeof(double);
			break;
		case GL_INT:
			data = (int*)data;
			t_size = sizeof(int);
			break;
		case GL_UNSIGNED_INT:
			data = (unsigned int*)data;
			t_size = sizeof(unsigned int);
			break;
		case GL_SHORT:
			data = (short*)data;
			t_size = sizeof(short);
			break;
		case GL_UNSIGNED_SHORT:
			data = (unsigned short*)data;
			t_size = sizeof(unsigned short);
			break;
		case GL_BYTE:
			data = (char*)data;
			t_size = sizeof(char);
			break;
		case GL_UNSIGNED_BYTE:
			data = (unsigned char*)data;
			t_size = sizeof(unsigned char);
			break;
		default:
			break;
		}

		GL_CALL(glBufferData(GL_ARRAY_BUFFER, dataSize * t_size * attribSize, data, usage));

		GLuint loc = glGetAttribLocation(programID, attribName.c_str());
		GL_CALL(glEnableVertexAttribArray(loc));

		GL_CALL(glVertexAttribPointer(loc, attribSize, type, normalized, stride, (void*)offset));
	}
};

struct VertexArrayObject
{
public:
	bool visible = true;
	
	/*
	* Creates and binds a VAO
	*/
	VertexArrayObject()
	{
		GL_CALL(glGenVertexArrays(1, &glID));
		GL_CALL(glBindVertexArray(glID));
		initialized = true;
	}

	bool IsInitialized() { return initialized; }
	unsigned int GetID() { return glID; }
	void SetDrawMode(GLenum mode) { drawMode = mode; }
	GLenum GetDrawMode() { return drawMode; }
	VertexBufferObject GetVBO(unsigned int index) { return VBOs[index]; }
	unsigned int GetNumVBOs() { return VBOs.size(); }
	unsigned int GetEBO()
	{
		if (EBO == -1)
			printf("EBO not initialized");
		return EBO;
	}
	/*
	* Binds VAO
	*/
	void Bind()
	{
		GL_CALL(glBindVertexArray(glID));
	}

	/*
	* Pushes a vertex buffer object to the VBO vector and binds it
	*/
	void AddVBO(VertexBufferObject& vbo) { VBOs.push_back(vbo); }

	/*
	* Creates a element buffer object and binds it
	*/
	void CreateEBO(unsigned int* indices, unsigned int count)
	{
		if (count > 0)
		{
			GL_CALL(glGenBuffers(1, &EBO));
			GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
			GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));

			numIndices = count;
		}
		else
			printf("Cannot create EBO. 0 indices\n");
	}
	/*
	* Deletes the VAO and all related buffers
	*/
	void Delete()
	{
		GL_CALL(glDeleteVertexArrays(1, &glID));
		for (unsigned int i = 0; i < VBOs.size(); i++)
		{
			GL_CALL(glDeleteBuffers(1, &VBOs[i].glID));
		}
		if (numIndices > 0)
			GL_CALL(glDeleteBuffers(1, &EBO));
		VBOs.clear();
	}

	/*
	* Deletes the selected VBO
	*/
	void DeleteVBO(unsigned int index)
	{
		GL_CALL(glDeleteBuffers(1, &VBOs[index].glID));
		VBOs.erase(VBOs.begin() + index);
	}

	/*
	* Deletes the EBO
	*/
	void DeleteEBO()
	{
		GL_CALL(glDeleteBuffers(1, &EBO));
		numIndices = 0;
	}

	/*
	* Draws the VAO using either glDrawArrays or glDrawArrays with the specified mode
	*/
	void Draw(GLenum mode)
	{
		if (!visible)
			return;

		if (VBOs.size() == 0)
		{
			//printf("No VBOs in VAO\n");
			return;
		}

		Bind();

		if (numIndices == 0)
		{
			GL_CALL(glDrawArrays(mode, 0, VBOs[0].dataSize));
		}
		else
		{
			GL_CALL(glDrawElements(mode, numIndices, GL_UNSIGNED_INT, 0));
		}
	}
	/*
	* Draws the VAO using either glDrawArrays or glDrawArrays with the set mode
	*/
	void Draw()
	{
		Draw(drawMode);
	}
	/*
	* Set the render type of the VAO which sets the uniform used in shaders
	*/
	/*void SetRenderType(RenderType type) { renderType = type; }
	RenderType GetRenderType() { return renderType; }*/

	void Update() {}

private:
	bool initialized = false;
	GLuint glID;
	std::vector<VertexBufferObject> VBOs;
	GLuint EBO;
	unsigned int numIndices = 0;
	GLenum drawMode = GL_TRIANGLES;
};

class OpenGLProgram
{
public:
	std::vector<VertexArrayObject> vaos;
	
	OpenGLProgram()
	{
		glID = glCreateProgram();
		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
		
		vertexShader = new Shader(GL_VERTEX_SHADER);
		fragmentShader = new Shader(GL_FRAGMENT_SHADER);
	};
	~OpenGLProgram()
	{
		GL_CALL(glDeleteProgram(glID));
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
	
	GLuint GetID() { return glID; }

	inline void SetUniform(const char* name, int value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform1i(location, value));
	}
	inline void SetUniform(const char* name, float value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform1f(location, value));
	}

	inline void SetUniform(const char* name, glm::vec2 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform2f(location, value.x, value.y));
	}
	inline void SetUniform(const char* name, glm::vec3 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform3f(location, value.x, value.y, value.z));
	}
	inline void SetUniform(const char* name, glm::vec4 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform4f(location, value.x, value.y, value.z, value.w));
	}

	inline void SetUniform(const char* name, glm::mat2 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]));
	}

	inline void SetUniform(const char* name, glm::mat3 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]));
	}

	inline void SetUniform(const char* name, glm::mat4 value)
	{
		GLint location = glGetUniformLocation(glID, name);
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]));
	}

	inline void SetGLClearFlags(GLbitfield flags)
	{
		clearFlags = flags;
	}
	inline void SetClearColor(glm::vec4 color)
	{
		clearColor = color;
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
	}

	inline void Clear()
	{
		GL_CALL(glClear(clearFlags));
	}
	
private:
	GLuint glID;
	Shader* vertexShader;
	Shader* fragmentShader;
	GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;
	glm::vec4 clearColor = glm::vec4(0.02f, 0.02f, 0.02f, 1.f);
};
