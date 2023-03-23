#include <OpenGLProgram.h>

void OpenGLProgram::Use()
{
	GL_CALL(glUseProgram(glID));
}

bool OpenGLProgram::AttachVertexShader()
{
	if (vertexShader->AttachShader(glID))
		GL_CALL(glDeleteShader(vertexShader->glID));//Lets drivers know we don't need this shader objects anymore.
	else 
		return false;
	
	return true;
}

bool OpenGLProgram::AttachFragmentShader()
{
	if (fragmentShader->AttachShader(glID))
		GL_CALL(glDeleteShader(fragmentShader->glID));//Lets drivers know we don't need this shader objects anymore.
	else
		return false;
	return true;
}

bool OpenGLProgram::AttachGeometryShader()
{
	if (!geometryShader)
		return false;
	if (geometryShader->AttachShader(glID))
		GL_CALL(glDeleteShader(geometryShader->glID));//Lets drivers know we don't need this shader objects anymore.
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

void OpenGLProgram::SetGeometryShaderSource(const char* src, bool compile)
{
	geometryShader->SetSource(src, compile);
}

void OpenGLProgram::SetVertexShaderSourceFromFile(const char* filePath, bool compile)
{
	vertexShader->SetSourceFromFile(filePath, compile);
}

void OpenGLProgram::SetFragmentShaderSourceFromFile(const char* filePath, bool compile)
{
	fragmentShader->SetSourceFromFile(filePath, compile);
}

void OpenGLProgram::SetGeometryShaderSourceFromFile(const char* filePath, bool compile)
{
	geometryShader->SetSourceFromFile(filePath, compile);
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

void OpenGLProgram::SetGeometryShader(Shader* shader)
{
	delete geometryShader;
	geometryShader = shader;
}

bool OpenGLProgram::CompileShaders()
{
	if (!vertexShader->Compile())
		return false;
	if (!fragmentShader->Compile())
		return false;
	if (geometryShader && !geometryShader->Compile())
		return false;
	return true;
};
