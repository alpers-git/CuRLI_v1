#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <Scene.h>
#include <windows.h>

//void APIENTRY GLDebugMessageCallback(
//	GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
//	const GLchar* msg, const void* data)
//{
//	std::string _source;
//	switch (source) {
//	case GL_DEBUG_SOURCE_API:               _source = "api"; break;
//	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     _source = "window"; break;
//	case GL_DEBUG_SOURCE_SHADER_COMPILER:   _source = "shader"; break;
//	case GL_DEBUG_SOURCE_THIRD_PARTY:       _source = "3rd party"; break;
//	case GL_DEBUG_SOURCE_APPLICATION:       _source = "app"; break;
//	case GL_DEBUG_SOURCE_OTHER:             _source = "UNKNOWN"; break;
//	default: _source = "UNKNOWN"; break;
//	}
//
//	std::string _type;
//	switch (type) {
//	case GL_DEBUG_TYPE_ERROR:               _type = "error"; break;
//	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: _type = "deprecated"; break;
//	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  _type = "undefined"; break;
//	case GL_DEBUG_TYPE_PORTABILITY:         _type = "portability"; break;
//	case GL_DEBUG_TYPE_PERFORMANCE:         _type = "performance"; break;
//	case GL_DEBUG_TYPE_OTHER:               _type = "other"; break;
//	case GL_DEBUG_TYPE_MARKER:              _type = "marker"; break;
//	default: _type = "UNKNOWN"; break;
//	}
//
//	std::string _severity;
//	//console color char for plain printf
//	
//	switch (severity) {
//	case GL_DEBUG_SEVERITY_HIGH:
//		_severity = "high";
//		system("color 4");
//		break;
//	case GL_DEBUG_SEVERITY_MEDIUM:
//		_severity = "med";
//		system("color 6");
//		break;
//	case GL_DEBUG_SEVERITY_LOW:
//		_severity = "low";
//		system("color 2");
//		break;
//	case GL_DEBUG_SEVERITY_NOTIFICATION:
//		_severity = "notif";
//		system("color 7");
//		break;
//	default:
//		_severity = "UNKNOWN";
//		system("color 1");
//		break;
//	}
//
//	if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
//		printf("GL Error type: %s severity: %s from: %s-\n\t %s", _type.c_str(), _severity.c_str(), _source.c_str(), msg);
//	}
//}

//define a macro that takes a function calls it and ctaches opengl errors
#define GL_CALL(func) \
do { \
    func; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        const char* errorString = (const char*)glad_glGetString(GL_VERSION); \
        const char* description = (const char*)glfwGetError(NULL); \
        printf("OpenGL error 0x%08x (%s) at %s:%d - %s\n", error, errorString, __FILE__, __LINE__, description); \
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
		GL_CALL(glID = glCreateShader(shader_type));
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
			GL_CALL(glGetShaderInfoLog(glID, 512, NULL, infoLog));
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
			GL_CALL(glGetProgramInfoLog(glID, 512, NULL, infoLog));
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

		GLuint loc;
		GL_CALL(loc = glGetAttribLocation(programID, attribName.c_str()));
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

struct Texture2D
{
public:
	Texture2D(void* data, glm::uvec2 dims, GLenum textUnit, GLenum wrapS = GL_REPEAT,
		GLenum wrapT = GL_REPEAT, GLenum dataType = GL_UNSIGNED_BYTE,
		GLenum format = GL_RGBA, int mipmapLevel = -1)
		:textUnit(textUnit), dataType(dataType), format(format),
		mipmapLevel(mipmapLevel), wrapS(wrapS), wrapT(wrapT)
	{
		GL_CALL(glGenTextures(1, &glID));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));

		GL_CALL(glTexImage2D(
			GL_TEXTURE_2D,
			mipmapLevel >= 0 ? mipmapLevel : 0,
			internalFormat,
			dims.x,
			dims.y,
			0,
			format,
			dataType,
			data==nullptr ? 0 : data));

		if (mipmapLevel >= 0)
			GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

		GL_CALL(glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			mipmapLevel >= 0 ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR));

		GL_CALL(glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			mipmapLevel >= 0 ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR));

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT));
	}
	
	Texture2D(const Texture2D& other)
	{
		glID = other.glID;
		textUnit = other.textUnit;
		dataType = other.dataType;
		format = other.format;
		mipmapLevel = other.mipmapLevel;
		wrapS = other.wrapS;
		wrapT = other.wrapT;
	}

	void SetParami(GLenum param, GLenum value)
	{
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, param, value));
	}
	
	void SetParamf(GLenum param, GLfloat value)
	{
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));
		GL_CALL(glTexParameterf(GL_TEXTURE_2D, param, value));
	}
	
	GLuint GetGLID() { return glID; }

	void Delete()
	{
		GL_CALL(glDeleteTextures(1, &glID));
	}

	void Bind()
	{
		GL_CALL(glActiveTexture(textUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));
	}

	void Unbind()
	{
		GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
	}

	int GetTextureUnitNum()
	{
		return textUnit - GL_TEXTURE0;
	}
	GLenum GetTextureUnit()
	{
		return textUnit;
	}

private:
	GLuint glID;
	GLenum wrapS, wrapT;
	GLenum format = GL_RGBA;
	int mipmapLevel = 0;
	GLenum dataType = GL_UNSIGNED_BYTE;
	GLenum internalFormat = GL_RGBA;

	GLenum textUnit = GL_TEXTURE0;
};

struct RenderedTexture2D
{
	RenderedTexture2D(glm::uvec2 dims, GLenum textUnit, bool hasDepthBuffer = true,
		GLenum wrapS = GL_REPEAT, GLenum wrapT = GL_REPEAT, 
		GLenum dataType = GL_UNSIGNED_BYTE, GLenum format = GL_RGB,
		int mipmapLevel = -1)
		:dims(dims), hasDepth(hasDepthBuffer),
		texture(nullptr, dims,textUnit, wrapS, wrapT, dataType, format, mipmapLevel)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));

		GL_CALL(glGenFramebuffers(1, &frameBufferID));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));

		if (hasDepthBuffer)
		{
			GL_CALL(glGenRenderbuffers(1, &depthBufferID));
			GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID));
			GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dims.x, dims.y));
		}
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID)); //For safety
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			GL_RENDERBUFFER, depthBufferID));
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.GetGLID(), 0));

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		GL_CALL(glDrawBuffers(1, drawBuffers));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
	}

	RenderedTexture2D(const RenderedTexture2D& other)
		:dims(other.dims), hasDepth(other.hasDepth),
		texture(other.texture), frameBufferID(other.frameBufferID), depthBufferID(other.depthBufferID)
	{}

	~RenderedTexture2D()
	{}

	void Render(std::function <void()> renderFunc)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));
		GLint origViewport[4];
		GL_CALL(glGetIntegerv(GL_VIEWPORT, origViewport));
		
		//Render the scene
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));
		GL_CALL(glViewport(0, 0, dims.x, dims.y));
		auto mask = GL_COLOR_BUFFER_BIT | (hasDepth ? GL_DEPTH_BUFFER_BIT : 0);
		GL_CALL(glClear(mask));
		renderFunc();//Tell how the scene is going to be rendered
		
		//Restore the renderer
		GL_CALL(glGenerateTextureMipmap(texture.GetGLID()));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
		GL_CALL(glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]));
		GL_CALL(glClear(mask));
	}

	Texture2D GetTexture()
	{
		return texture;
	}
	
	void Delete()
	{
		texture.Delete();
		GL_CALL(glDeleteFramebuffers(1, &frameBufferID));
		if (hasDepth)
			GL_CALL(glDeleteRenderbuffers(1, &depthBufferID));
	}
	
	Texture2D texture;
	GLuint frameBufferID=0;
	GLuint depthBufferID = 0;
	glm::uvec2 dims;
	bool hasDepth;
};

struct ShadowTexture
{
	ShadowTexture(glm::uvec2 dims, GLenum texUnit = GL_TEXTURE10)
		:dims(dims), texUnit(texUnit)
	{
		//configure depth texture
		GL_CALL(glGenTextures(1, &glID));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));
		
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 
			0, GL_DEPTH_COMPONENT24,
			dims.x, dims.y,
			0, GL_DEPTH_COMPONENT,
			GL_FLOAT, 0));

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		
		//save the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));

		//configure framebuffer
		GL_CALL(glGenFramebuffers(1, &frameBufferID));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, glID, 0););//different
		
		GL_CALL(glDrawBuffer(GL_NONE));
		GL_CALL(glReadBuffer(GL_NONE));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		//preserve render state
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
	}
	
	~ShadowTexture()
	{}
	
	void Render(std::function <void()> renderFunc)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));
		GLint origViewport[4];
		GL_CALL(glGetIntegerv(GL_VIEWPORT, origViewport));

		//Render the scene
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));
		GL_CALL(glViewport(0, 0, dims.x, dims.y));
		auto mask = GL_DEPTH_BUFFER_BIT;
		GL_CALL(glClear(mask));
		//glEnable(GL_CULL_FACE);
		GL_CALL(glCullFace(GL_FRONT));
		renderFunc();//Tell how the scene is going to be rendered
		GL_CALL(glCullFace(GL_BACK));
		GL_CALL(glDisable(GL_CULL_FACE));

		//Restore the renderer
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
		GL_CALL(glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]));
		GL_CALL(glClear(mask));
	}

	void Bind()
	{
		GL_CALL(glActiveTexture(texUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, glID));
	}

	void Delete()
	{
		GL_CALL(glDeleteFramebuffers(1, &frameBufferID));
		GL_CALL(glDeleteRenderbuffers(1, &depthBufferID));
	}

	GLuint GetGLID()
	{
		return glID;
	}

private:
	GLuint frameBufferID;
	GLuint depthBufferID;
	GLuint glID;
	glm::uvec2 dims;
	
	GLuint texUnit;
	
};

struct CubeMappedTexture
{
	CubeMappedTexture(void* data, glm::uvec2 dims, bool seamless = true,
		GLenum textUnit= GL_TEXTURE30, GLenum wrapS = GL_CLAMP_TO_EDGE,
		GLenum wrapT = GL_CLAMP_TO_EDGE, GLenum dataType = GL_UNSIGNED_BYTE,
		GLenum format = GL_RGBA, int mipmapLevel = -1)
		:dims(dims), seamless(seamless), textUnit(textUnit),
		wrapS(wrapS), wrapT(wrapT), dataType(dataType), format(format),
		mipmapLevel(mipmapLevel)
	{
		GL_CALL(glGenTextures(1, &glID));
		
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, glID));

		size_t typeSize = 0;
		
		switch (dataType)
		{
		case GL_UNSIGNED_BYTE:
			typeSize = 4;
			break;
		case GL_UNSIGNED_SHORT:
			typeSize = 2;
			break;
		case GL_UNSIGNED_INT:
			typeSize = sizeof(GLuint);
			break;
		case GL_FLOAT:
			typeSize = sizeof(GLfloat);
			break;
		default:
			std::cout << "ERROR::CUBEMAP:: Unknown data type" << std::endl;
			break;
		}

		for (size_t i = 0; i < 6; i++)
		{
			//Create a temporary array that is a slice of data to each of 6 pieces using dims
			void* tmp = nullptr;
			if (data != nullptr)
				tmp = (char*)data + i * dims.x * dims.y * typeSize;
			
			GL_CALL(glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				mipmapLevel >= 0 ? mipmapLevel : 0,
				internalFormat,
				dims.x,
				dims.y,
				0,
				format,
				dataType,
				data == nullptr ? 0 : tmp));
		}
		if(seamless)
			GL_CALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
			
		if (mipmapLevel > 0)
			GL_CALL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

		GL_CALL(glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			mipmapLevel >= 0 ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR));

		GL_CALL(glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR));

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT));

	}

	CubeMappedTexture(glm::uvec2 dims, bool seamless = true,
		GLenum textUnit = GL_TEXTURE30, GLenum wrapS = GL_CLAMP_TO_EDGE,
		GLenum wrapT = GL_CLAMP_TO_EDGE, GLenum dataType = GL_UNSIGNED_BYTE,
		GLenum format = GL_RGBA, int mipmapLevel = -1) // For rendered cubemap variant
		: CubeMappedTexture(nullptr, dims, seamless, textUnit, wrapS, 
			wrapT, dataType, format, mipmapLevel)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));

		GL_CALL(glGenFramebuffers(1, &frameBufferID));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));

		GL_CALL(glGenRenderbuffers(1, &depthBufferID));
		GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID));
		GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dims.x, dims.y));
		
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID)); //For safety
		GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, depthBufferID));
		GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glID , 0));

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		GL_CALL(glDrawBuffers(1, drawBuffers));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
	}

	CubeMappedTexture(const CubeMappedTexture& other)
		:glID(other.glID), dims(other.dims), seamless(other.seamless), textUnit(other.textUnit),
		wrapS(other.wrapS), wrapT(other.wrapT), dataType(other.dataType),
		format(other.format), mipmapLevel(other.mipmapLevel), frameBufferID(other.frameBufferID),
		depthBufferID(other.depthBufferID)
	{}

	~CubeMappedTexture()
	{}

	void RenderSide(int i, std::function <void()> renderFunc, bool lastFace = false)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));
		GLint origViewport[4];
		GL_CALL(glGetIntegerv(GL_VIEWPORT, origViewport));

		//Render the scene
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));
		GL_CALL(glViewport(0, 0, dims.x, dims.y));
		auto mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, glID, 0));
		if(!lastFace)
			GL_CALL(glClear(mask));
		renderFunc();//Tell how the scene is going to be rendered

		//Restore the renderer
		GL_CALL(glGenerateTextureMipmap(glID));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
		GL_CALL(glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]));
		if (lastFace)
			GL_CALL(glClear(mask));
	}

	void RenderAll(std::function <void()> renderFunc)
	{
		for (int i = 0; i < 6; i++)
			RenderSide(i, renderFunc);
	}

	void Bind()
	{
		GL_CALL(glActiveTexture(textUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, glID));
	}

	void Unbind()
	{
		GL_CALL(glActiveTexture(textUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	void Delete()
	{
		GL_CALL(glDeleteTextures(1, &glID));
	}

	GLuint GetGLID()
	{
		return glID;
	}

	glm::uvec2 GetDims()
	{
		return dims;
	}
	

private:
	GLuint glID;
	glm::uvec2 dims;
	GLenum wrapS, wrapT;
	GLenum format = GL_RGBA;
	int mipmapLevel = 0;
	GLenum dataType = GL_UNSIGNED_BYTE;
	GLenum internalFormat = GL_RGBA;

	bool seamless;

	GLenum textUnit;

	GLuint frameBufferID = 0;
	GLuint depthBufferID = 0;
};

struct ShadowCubeTexture
{
public:
	ShadowCubeTexture(glm::uvec2 dims, GLenum textUnit = GL_TEXTURE10)
		:dims(dims), textUnit(textUnit)
	{
		GL_CALL(glGenTextures(1, &glID));
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, glID));

		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));
		
		GL_CALL(glGenFramebuffers(1, &frameBufferID));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));

		GL_CALL(glDrawBuffer(GL_NONE));
		GL_CALL(glReadBuffer(GL_NONE));

		for (unsigned int i = 0; i < 6; ++i)
		{
			GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
				dims.x, dims.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
			
			GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, glID, 0));
		}


		//Get the renderer state
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		}
		//Rebind the scene FB
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
	}
	
	//Copy constructor
	ShadowCubeTexture(const ShadowCubeTexture& other)
	{
		glID = other.glID;
		dims = other.dims;
		textUnit = other.textUnit;
		frameBufferID = other.frameBufferID;
		depthBufferID = other.depthBufferID;
	}
	~ShadowCubeTexture()
	{}

	void RenderSide(int i, std::function <void()> renderFunc, bool lastFace = false)
	{
		//Get the renderer state
		GLint origFB;
		GL_CALL(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFB));
		GLint origViewport[4];
		GL_CALL(glGetIntegerv(GL_VIEWPORT, origViewport));

		//Render the scene
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID));
		GL_CALL(glViewport(0, 0, dims.x, dims.y));
		auto mask = GL_DEPTH_BUFFER_BIT;
		
		GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, glID, 0));
		/*if (!lastFace)*/
			//GL_CALL(glClear(mask));
		renderFunc();//Tell how the scene is going to be rendered

		//Restore the renderer
		//GL_CALL(glGenerateTextureMipmap(glID));
		if (lastFace)
			GL_CALL(glClear(mask));
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, origFB));
		GL_CALL(glViewport(origViewport[0], origViewport[1], origViewport[2], origViewport[3]));
	}

	void RenderAll(std::function <void()> renderFunc)
	{
		for (int i = 0; i < 6; i++)
			RenderSide(i, renderFunc);
	}

	void Bind()
	{
		GL_CALL(glActiveTexture(textUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, glID));
	}

	void Unbind()
	{
		GL_CALL(glActiveTexture(textUnit));
		GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
	}

	void Delete()
	{
		GL_CALL(glDeleteTextures(1, &glID));
	}

	GLuint GetGLID()
	{
		return glID;
	}

	glm::uvec2 GetDims()
	{
		return dims;
	}
	
	
		
private:
	GLuint glID;
	glm::uvec2 dims;
	
	GLenum dataType = GL_UNSIGNED_BYTE;

	GLenum textUnit;

	GLuint frameBufferID = 0;
	GLuint depthBufferID = 0;
};

class OpenGLProgram
{
public:
	std::vector<VertexArrayObject> vaos;
	std::vector<Texture2D> textures;
	std::vector<RenderedTexture2D> renderedTextures;
	std::vector<ShadowTexture> shadowTextures;
	std::vector<CubeMappedTexture> cubeMaps;
	std::vector<ShadowCubeTexture> shadowCubeMaps;
	
	OpenGLProgram()
	{
		GL_CALL(glID = glCreateProgram());
		GL_CALL(glEnable(GL_DEPTH_TEST));
		GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));

		glDebugMessageCallback(//create empty lambda here
			[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
			{
				std::string _source;
				switch (source) {
				case GL_DEBUG_SOURCE_API:               _source = "api"; break;
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     _source = "window"; break;
				case GL_DEBUG_SOURCE_SHADER_COMPILER:   _source = "shader"; break;
				case GL_DEBUG_SOURCE_THIRD_PARTY:       _source = "3rd party"; break;
				case GL_DEBUG_SOURCE_APPLICATION:       _source = "app"; break;
				case GL_DEBUG_SOURCE_OTHER:             _source = "UNKNOWN"; break;
				default: _source = "UNKNOWN"; break;
				}

				std::string _type;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:               _type = "error"; break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: _type = "deprecated"; break;
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  _type = "undefined"; break;
				case GL_DEBUG_TYPE_PORTABILITY:         _type = "portability"; break;
				case GL_DEBUG_TYPE_PERFORMANCE:         _type = "performance"; break;
				case GL_DEBUG_TYPE_OTHER:               _type = "other"; break;
				case GL_DEBUG_TYPE_MARKER:              _type = "marker"; break;
				default: _type = "UNKNOWN"; break;
				}

				std::string _severity;
				//console color char for plain printf

				switch (severity) {
				case GL_DEBUG_SEVERITY_HIGH:
					_severity = "high";
					//system("color 4");
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:
					_severity = "med";
					//system("color 6");
					break;
				case GL_DEBUG_SEVERITY_LOW:
					_severity = "low";
					//system("color 2");
					break;
				case GL_DEBUG_SEVERITY_NOTIFICATION:
					_severity = "notif";
					//system("color 7");
					break;
				default:
					_severity = "UNKNOWN";
					//system("color 1");
					break;
				}

				if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
					printf("GL Error type: %s severity: %s from: %s-\n\t %s", _type.c_str(), _severity.c_str(), _source.c_str(), message);
				}
				//system("color 7");
			}, 0);
		
		vertexShader = new Shader(GL_VERTEX_SHADER);
		fragmentShader = new Shader(GL_FRAGMENT_SHADER);
	};
	~OpenGLProgram()
	{
		GL_CALL(glDeleteProgram(glID));
		delete vertexShader;
		delete fragmentShader;
		if (geometryShader)
			delete geometryShader;
	};

	//void CreateProgram();
	void Use();
	//void DeleteProgram();
	
	/*
	* Read compile and attach shaders to the opengl program.
	*/
	bool CreatePipelineFromFiles(const char* filePathVert, const char* filePathFrag,
		const char* filePathGeom = nullptr, const char* filePathTessControl = nullptr,
		const char* filePathTessEval = nullptr, int patchSize = 3);
	
	/*
	* Assuming shader sources are already set this function will
	compile and attach shaders to the opengl program.
	*/
	bool CreatePipeline()
	{
		if (!CompileShaders())
			return false;

		AttachVertexShader();
		AttachGeometryShader();
		AttachTessellationShaders();
		AttachFragmentShader();
	}
	

	bool AttachVertexShader();
	bool AttachFragmentShader();
	bool AttachGeometryShader();
	bool AttachTessellationShaders(int patchSize = 3);

	void SetVertexShaderSource(const char* src, bool compile = false);
	void SetFragmentShaderSource(const char* src, bool compile = false);
	void SetGeometryShaderSource(const char* src, bool compile = false);
	void SetTessellationShaderSources(const char* cSrc, const char* eSrc, bool compile = false);

	void SetVertexShaderSourceFromFile(const char* filePath, bool compile = false);
	void SetFragmentShaderSourceFromFile(const char* filePath, bool compile = false);
	void SetGeometryShaderSourceFromFile(const char* filePath, bool compile = false);
	void SetTessellationShaderSourcesFromFiles(const char* cPath, const char* ePath, bool compile = false);
	
	void SetVertexShader(Shader* shader);
	void SetFragmentShader(Shader* shader);
	void SetGeometryShader(Shader* shader);
	void SetTessellationShaders(Shader* cShader, Shader* eSHader);
	
	bool CompileShaders();
	
	GLuint GetID() { return glID; }

	inline void SetUniform(const char* name, int value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform1i(location, value));
	}
	inline void SetUniform(const char* name, float value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform1f(location, value));
	}

	inline void SetUniform(const char* name, glm::vec2 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform2f(location, value.x, value.y));
	}
	inline void SetUniform(const char* name, glm::vec3 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform3f(location, value.x, value.y, value.z));
	}
	inline void SetUniform(const char* name, glm::vec4 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniform4f(location, value.x, value.y, value.z, value.w));
	}

	inline void SetUniform(const char* name, glm::mat2 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniformMatrix2fv(location, 1, GL_FALSE, &value[0][0]));
	}

	inline void SetUniform(const char* name, glm::mat3 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
		if (location == -1)
		{
			std::cout << "ERROR::SHADER::UNIFORM::" << name << "::NOT_FOUND" << std::endl;
			return;
		}
		GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, &value[0][0]));
	}

	inline void SetUniform(const char* name, glm::mat4 value)
	{
		GLint location;
		GL_CALL(location = glGetUniformLocation(glID, name));
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
	
	inline glm::vec4 GetClearColor()
	{
		return clearColor;
	}

	inline void Clear()
	{
		GL_CALL(glClear(clearFlags));
	}
	
	
private:
	GLuint glID;
	Shader* vertexShader;
	Shader* fragmentShader;
	Shader* geometryShader = nullptr; //optional
	Shader* tessControlShader = nullptr; //optional
	Shader* tessEvalShader = nullptr; //optional
	GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;
	glm::vec4 clearColor = glm::vec4(0.02f, 0.02f, 0.02f, 1.f);
};
