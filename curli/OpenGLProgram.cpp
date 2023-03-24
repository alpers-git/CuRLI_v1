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

bool OpenGLProgram::AttachTessellationShaders(int patchSize)
{
	if (!tessControlShader || !tessEvalShader)
		return false;
	if (tessControlShader->AttachShader(glID))
		GL_CALL(glDeleteShader(tessControlShader->glID));//Lets drivers know we don't need this shader objects anymore.
	else
		return false;
	if (tessEvalShader->AttachShader(glID))
		GL_CALL(glDeleteShader(tessEvalShader->glID));//Lets drivers know we don't need this shader objects anymore.
	else
		return false;
	GL_CALL(glPatchParameteri(GL_PATCH_VERTICES, patchSize));
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
	if (!geometryShader)
		geometryShader = new Shader(GL_GEOMETRY_SHADER);
	geometryShader->SetSource(src, compile);
}

void OpenGLProgram::SetTessellationShaderSources(const char* srcC, const char* srE, bool compile)
{
	if (!tessControlShader)
		tessControlShader = new Shader(GL_TESS_CONTROL_SHADER);
	tessControlShader->SetSource(srcC, compile);
	if (!tessEvalShader)
		tessEvalShader = new Shader(GL_TESS_EVALUATION_SHADER);
	tessEvalShader->SetSource(srE, compile);
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
	if (!geometryShader)
		geometryShader = new Shader(GL_GEOMETRY_SHADER);
	geometryShader->SetSourceFromFile(filePath, compile);
}

void OpenGLProgram::SetTessellationShaderSourcesFromFiles(const char* filePathC, const char* filePathE, bool compile)
{
	if (!tessControlShader)
		tessControlShader = new Shader(GL_TESS_CONTROL_SHADER);
	tessControlShader->SetSourceFromFile(filePathC, compile);
	if (!tessEvalShader)
		tessEvalShader = new Shader(GL_TESS_EVALUATION_SHADER);
	tessEvalShader->SetSourceFromFile(filePathE, compile);
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

void OpenGLProgram::SetTessellationShaders(Shader* controlShader, Shader* evalShader)
{
	delete tessControlShader;
	delete tessEvalShader;
	tessControlShader = controlShader;
	tessEvalShader = evalShader;
}

bool OpenGLProgram::CompileShaders()
{
	if (!vertexShader->Compile())
		return false;
	if (!fragmentShader->Compile())
		return false;
	if (geometryShader && !geometryShader->Compile())
		return false;
	if (tessControlShader && !tessControlShader->Compile())
		return false;
	if (tessEvalShader && !tessEvalShader->Compile())
		return false;
	return true;
};
