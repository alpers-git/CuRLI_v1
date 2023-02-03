#include <OpenGLProgram.h>

void OpenGLProgram::Use()
{
	glUseProgram(glID);
}

bool OpenGLProgram::AttachVertexShader()
{
	if (vertexShader->AttachShader(glID))
		glDeleteShader(vertexShader->glID);//Lets drivers know we don't need this shader objects anymore.
	else 
		return false;
	
	return true;
}

bool OpenGLProgram::AttachFragmentShader()
{
	if (fragmentShader->AttachShader(glID))
		glDeleteShader(fragmentShader->glID);//Lets drivers know we don't need this shader objects anymore.
	else
		return false;
	return true;
}

void OpenGLProgram::SetVertexShaderSource(const char* src, bool compile)
{
	vertexShader->SetSource(src, compile);
}

void OpenGLProgram::SetFragmentShaderSource(const char* src, bool compile)
{
	fragmentShader->SetSource(src, compile);
}

void OpenGLProgram::SetVertexShaderSourceFromFile(const char* filePath, bool compile)
{
	vertexShader->SetSourceFromFile(filePath, compile);
}

void OpenGLProgram::SetFragmentShaderSourceFromFile(const char* filePath, bool compile)
{
	fragmentShader->SetSourceFromFile(filePath, compile);
}

void OpenGLProgram::SetVertexShader(Shader* shader)
{
	delete vertexShader;
	vertexShader = shader;
}

void OpenGLProgram::SetFragmentShader(Shader* shader)
{
	delete fragmentShader;
	fragmentShader = shader;
}

bool OpenGLProgram::CompileShaders()
{
	if (!vertexShader->Compile())
		return false;
	if (!fragmentShader->Compile())
		return false;
	return true;
};
