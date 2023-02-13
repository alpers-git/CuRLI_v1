#pragma once
#include <glad/glad.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <cyTriMesh.h>
#include <CyToGLMHelper.h>
#include <glm/gtx/log_base.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <stdarg.h>
#include <string>
#include <map>

struct Camera
{
public:
	Camera(glm::vec3 center = { 0.f,0.f,0.f }, glm::vec3 eye = { 10.f,0.f,0.f }, glm::vec3 up={0.f, 1.f, 0.f},
		float fov = 45.f, float near = 0.1f, float far = 100.f,
		float aspectRatio = 1.f, bool perspective = true)
		:center(center), eye(eye), up(up),
		fov(fov), nearPlane(near), farPlane(far), 
		aspectRatio(aspectRatio), perspective(perspective)
	{
		this->CalculateViewMatrix();
		this->CalculateProjectionMatrix();
	}
	Camera(glm::vec3 center, glm::vec3 angles, float distance,
		float fov = 45.f, float near = 0.1f, float far = 100.f,
		float aspectRatio = 1.f, bool perspective = true)
		:center(center),
		fov(fov), nearPlane(near), farPlane(far),
		aspectRatio(aspectRatio), perspective(perspective)
	{
		SetOrbitAngles(angles);
		SetOrbitDistance(distance);
		this->CalculateViewMatrix();
		this->CalculateProjectionMatrix();
	}
	
	glm::vec3 GetCenter() { return center; }
	
	glm::vec3 GetLookAtEye() { return eye; }
	glm::vec3 GetLookAtUp() { return up; }
	
	glm::vec3 GetOrbitAngles() 
	{ 
		return glm::vec3(
			glm::degrees(asin((eye.y - center.y) / glm::distance(eye, center))),
			glm::degrees(atan2f((-eye.z + center.z), (eye.x - center.x)))+90,
			0.f);
	}
	float GetOrbitDistance() { return glm::length(eye - center); }
	
	float GetFOV() { return fov; }
	float GetAspectRatio() { return aspectRatio; }
	float GetNearPlane() { return nearPlane; }
	float GetFarPlane() { return farPlane; }
	bool IsPerspective() { return perspective; }
	
	glm::mat4 GetViewMatrix()
	{
		if (viewDirty)
			CalculateViewMatrix();
		return viewMatrix;
	}
	glm::mat4 GetProjectionMatrix()
	{
		if (projectionDirty)
			CalculateProjectionMatrix();
		return projectionMatrix;
	}
	
	void inline SetCenter(glm::vec3 center, bool recalculate = false)
	{
		this->center = center;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	
	void inline SetLookAtUp(glm::vec3 up, bool recalculate = false)
	{
		this->up = up;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void inline SetLookAtEye(glm::vec3 eye, bool recalculate = false)
	{
		this->eye = eye;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	

	void inline SetOrbitAngles(glm::vec3 angles, bool recalculate = false)
	{
		if (angles.x > 89.5f)
			angles.x = 89.5f;
		if (angles.x < -89.5f)
			angles.x = -89.5f;
		const float theta = glm::radians(angles.x);
		const float phi = glm::radians(angles.y);
		glm::vec3 unitSpherePos = {
			cos(theta) * sin(phi),
			sin(theta),
			cos(theta) * cos(phi)
		};
		eye = center + unitSpherePos * glm::length(eye-center);
		up = {0.f, 1.f, 0.f};
		
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void inline SetOrbitDistance(float distance, bool recalculate = false)
	{
		eye = center + glm::normalize(eye - center) * glm::max(distance, 0.001f);
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	
	//--------------------------------------------------------------------//
	void inline SetPerspective(bool perspective, bool recalculate = false)
	{
		this->perspective = perspective;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetFOV(float fov, bool recalculate = false)
	{
		this->fov = fov;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetAspectRatio(float aspectRatio, bool recalculate = false)
	{
		this->aspectRatio = aspectRatio;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetNearPlane(float nearPlane, bool recalculate = false)
	{
		this->nearPlane = nearPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetFarPlane(float farPlane, bool recalculate = false)
	{
		this->farPlane = farPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	
private:
	glm::vec3 eye = glm::vec3(1.f);
	glm::vec3 center = glm::vec3(0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 angles;
	
	float fov = 45.f;
	float aspectRatio = 1.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	bool perspective = true;
	
	bool viewDirty = false;
	bool projectionDirty = false;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	
	void inline CalculateViewMatrix()
	{
		viewMatrix = glm::lookAt(eye, center, up);
		viewDirty = false;
	}
	void inline CalculateProjectionMatrix()
	{
		if (perspective)
			projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		else
			projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, nearPlane, farPlane);

		projectionDirty = false;
	}
};

struct Component
{
	virtual void Update() = 0;
};

struct CTransform : Component
{
public:
	CTransform(glm::vec3 position = glm::vec3(0.f), glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f))
		: position(position), rotation(rotation), scale(scale)
	{
		//pivot = position;
		CalculateModelMatrix();
	}

	//getter and setters
	void SetPosition(glm::vec3 position, bool recalculate = false)
	{
		this->position = position;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetEulerRotation(glm::vec3 rotation, bool recalculate = false)
	{
		this->rotation = rotation;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetScale(glm::vec3 scale, bool recalculate = false)
	{
		this->scale = scale;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetPivot(glm::vec3 pivot, bool recalculate = false)
	{
		this->pivot = pivot;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Translate(glm::vec3 offset, bool recalculate = false)
	{
		position += offset;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Rotate(glm::vec3 angles, bool recalculate = false)
	{
		this->rotation += angles;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Scale(glm::vec3 multiplier, bool recalculate = false)
	{
		this->scale *= multiplier;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	glm::vec3 GetPosition() { return position; }
	glm::vec3 GetPivot() { return pivot; }
	glm::vec3 GetRotation() { return rotation; }
	glm::vec3 GetScale() { return scale; }
	glm::mat4 GetModelMatrix()
	{
		if (modelDirty)
			CalculateModelMatrix();
		return modelMatrix;
	}
	
	void CalculateModelMatrix()
	{
		modelMatrix = glm::translate(glm::mat4(1.f), position);
		modelMatrix = modelMatrix * glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
		modelMatrix = glm::scale(modelMatrix, scale);
		modelMatrix = glm::translate(modelMatrix, -pivot);//everything happens with respect to pivot

		modelDirty = false;
	}

	void Update();
	
private:
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 pivot = glm::vec3(0.f);//rotation and scaling pivot point

	glm::mat4 modelMatrix = glm::mat4(1.f);
	bool modelDirty = false;

};

struct CTriMesh : Component
{
public:
	CTriMesh(cy::TriMesh& mesh)
		:mesh(mesh)
	{}

	CTriMesh(const std::string& path)
		:mesh(cy::TriMesh())
	{
		LoadObj(path.c_str());
	}
	
	CTriMesh()
		:mesh(cy::TriMesh())
	{}
	
	cy::TriMesh& GetMesh() { return mesh; }
	
	unsigned int GetNumVertices() { return mesh.NV(); }
	unsigned int GetNumFaces() { return mesh.NF(); }
	unsigned int GetNumNormals() { return mesh.NVN(); }
	unsigned int GetNumTextureVertices() { return mesh.NVT(); }
	
	glm::vec3 GetVertex(unsigned int index) { return glm::cy2GLM(mesh.V(index)); }
	glm::vec3 GetVNormal(unsigned int index) { return glm::cy2GLM(mesh.VN(index)); }
	glm::vec3 GetVTexture(unsigned int index) { return glm::cy2GLM(mesh.VT(index)); }
	
	glm::ivec3 GetFNormal(unsigned int index) { 
		const auto tmp = mesh.FN(index);
		return glm::ivec3(tmp.v[0], tmp.v[1], tmp.v[2]);
	}
	glm::ivec3 GetFace(unsigned int index) { 
		const auto tmp = mesh.F(index);
		return glm::ivec3(tmp.v[0], tmp.v[1], tmp.v[2]);
	}
	glm::ivec3 GetFTexture(unsigned int index) {
		const auto tmp = mesh.FT(index);
		return glm::ivec3(tmp.v[0], tmp.v[1], tmp.v[2]);
	}

	void* GetVertexDataPtr() { return &mesh.V(0); }
	void* GetNormalDataPtr() { return &mesh.VN(0); }
	void* GetTextureDataPtr() { return &mesh.VT(0); }
	void* GetFaceDataPtr() { return &mesh.F(0); }

	
	
	
	//glm::vec3 GetFaceNormal(unsigned int index) { return glm::cy2GLM(mesh.FN(index)); }

	inline void ComputeBoundingBox()
	{
		mesh.ComputeBoundingBox();
		bBoxInitialized = true;
	}

	inline void ComputeNormals()
	{
		mesh.ComputeNormals();
	}
	
	inline glm::vec3 GetBoundingBoxMin()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return glm::cy2GLM(mesh.GetBoundMin());
	}

	inline glm::vec3 GetBoundingBoxMax()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return glm::cy2GLM(mesh.GetBoundMax());
	}

	inline glm::vec3 GetBoundingBoxCenter()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return glm::cy2GLM(mesh.GetBoundMax() + mesh.GetBoundMin()) * .5f;
	}

	inline void LoadObj(const std::string& path)
	{
		mesh.LoadFromFileObj(path.c_str());
		printf("Loaded model with %d vertices, vertex normals %d, texture vertices %d, and faces %d from %s\n",
			GetNumVertices(),
			GetNumNormals(),
			GetNumTextureVertices(),
			GetNumFaces(),
			path.c_str());
	}

	void Update();
	
private:
	cy::TriMesh mesh;
	bool bBoxInitialized = false;
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
		glGenBuffers(1, &glID);
		glBindBuffer(GL_ARRAY_BUFFER, glID);
		
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

		glBufferData(GL_ARRAY_BUFFER, dataSize * t_size * attribSize, data, usage);

		GLuint loc = glGetAttribLocation(programID, attribName.c_str());
		glEnableVertexAttribArray(loc);
		
		glVertexAttribPointer(loc, attribSize, type, normalized, stride, (void*)offset);
	}
};
struct CVertexArrayObject : Component
{
public:
	bool visible = true;
	CVertexArrayObject() : glID(-1), EBO(-1) //will pop up as a very high value as these are uints
	{}

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
	* Creates and binds a VAO
	*/
	void CreateVAO()
	{
		glGenVertexArrays(1, &glID);
		glBindVertexArray(glID);
	}
	/*
	* Binds VAO
	*/
	void Bind()
	{
		glBindVertexArray(glID);
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
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
		
		numIndices = count;
	}
	/*
	* Deletes the VAO and all related buffers
	*/
	void Delete()
	{
		glDeleteVertexArrays(1, &glID);
		for (unsigned int i = 0; i < VBOs.size(); i++)
		{
			glDeleteBuffers(1, &VBOs[i].glID);
		}
		if (numIndices > 0)
			glDeleteBuffers(1, &EBO);
	}
	
	/*
	* Deletes the selected VBO
	*/
	void DeleteVBO(unsigned int index)
	{
		glDeleteBuffers(1, &VBOs[index].glID);
		VBOs.erase(VBOs.begin() + index);
	}
	
	/*
	* Deletes the EBO
	*/
	void DeleteEBO()
	{
		glDeleteBuffers(1, &EBO);
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
			printf("No VBOs in VAO\n");
			return;
		}
		Bind();
		
		if (numIndices == 0)
		{
			glDrawArrays(mode, 0, VBOs[0].dataSize);
		}
		else
		{
			glDrawElements(mode, numIndices, GL_UNSIGNED_INT, 0);
		}
	}
	/*
	* Draws the VAO using either glDrawArrays or glDrawArrays with the set mode
	*/
	void Draw()
	{
		Draw(drawMode);
	}
	

	void Update() {}
	
private:
	GLuint glID;
	std::vector<VertexBufferObject> VBOs;
	GLuint EBO;
	unsigned int numIndices = 0;
	GLenum drawMode = GL_TRIANGLES;
};

enum class LightType
{
	POINT,
	DIRECTIONAL,
	SPOT
};
struct CLight : Component
{
public:
	CLight(LightType type, glm::vec3 color, float intensity, glm::vec3 position,
		glm::vec3 direction, float iCutoff, float oCutoff)
		:lightType(type)
	{
		this->color = color;
		this->intensity = intensity;
		//Point light constructor
		if (type == LightType::POINT)
		{
			this->position = position;
			//invalid values for safety
			direction = glm::vec3(NAN, NAN, NAN);
			innerCutOff = -1;
			outerCutoff = -1;
		}
		//Directional light constructor
		else if (type == LightType::DIRECTIONAL)
		{
			this->direction = direction;
			lightType = LightType::DIRECTIONAL;

			//invalid values for safety
			position = glm::vec3(NAN, NAN, NAN);
			innerCutOff = -1;
			outerCutoff = -1;
		}
		//Spot light constructor
		else if (type == LightType::SPOT)
		{
			this->position = position;
			this->direction = direction;

			innerCutOff = iCutoff;
			outerCutoff = oCutoff;
		}
		
	}
	
	LightType GetLightType() { return lightType; }
	
	glm::vec3 color;
	float intensity;
	float innerCutOff;
	float outerCutoff;
	glm::vec3 direction;
	glm::vec3 position;
	
	void Update();
	
private:
	LightType lightType;
	
};

struct CRigidBody : Component
{
public:
	CRigidBody(float mass, glm::vec3 position, glm::vec3 rotation, float drag = 0.0f)
	{
		this->mass = mass;
		this->position = position;
		this->rotation = rotation;
		drag = drag;
	}

	float mass = 0.0f;
	float drag = 0.0f;
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 velocity = glm::vec3(0, 0, 0);
	glm::vec3 acceleration = glm::vec3(0, 0, 0);

	void ApplyForce(glm::vec3 force);

	void Update();
	
private:
};

struct CPhongMaterial : Component
{
public:
	glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.3f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(0.9f, 0.9f, 0.9f);
	float shininess = 450.f;

	void Update();
};

enum class FieldPlane
{
	XY, YZ, XZ
};
struct CVelocityField2D : Component
{
public:
	
	void Update();
	//function pointer here
	std::function<glm::vec2 (glm::vec2)> field;
	float scaling = 1.f;
	
	FieldPlane plane;
	
	CVelocityField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{}

	glm::vec3 VelocityAt(glm::vec2 p);
};

struct CForceField2D : Component
{
public:
	void Update();
	//function pointer here
	std::function<glm::vec2(glm::vec2)> field;
	float scaling = 1.f;
	FieldPlane plane;

	CForceField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{}

	glm::vec3 ForceAt(glm::vec2 p);
};

struct CBoundingBox : Component
{
public:
	CBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		this->min = glm::min(min, max);
		this->max = glm::max(min, max);
	}

	glm::vec3 min;
	glm::vec3 max;

	void Update();
	void Rebound(CRigidBody& rigidBody);
};

class Scene
{
public:
	Scene();
	~Scene();

	entt::entity CreateSceneObject(std::string name);
	//void DestroyEntity(entt::entity entity);

	void Update();
	
	/*
	* Returns a reference to the specified component of the specified entity. 
	* Use empty entity to fetch the first entity with the specified component
	*/
	template <typename C>
	C& GetComponent(entt::entity entity)
	{
		return registry.view<C>().get<C>(entity);
	};

	/*
	* Reads an obj file and adds and entity to the sceneObjects with a transform, phong material and a mesh
	*/
	entt::entity CreateModelObject(const std::string& meshPath, glm::vec3 position = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));
	/*
	* Adds and entity to the sceneObjects with a transform, phong material and a mesh
	*/
	entt::entity CreateModelObject(cy::TriMesh& mesh, glm::vec3 position = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));
	/*
	* Creates a Point light source
	*/
	entt::entity CreatePointLight(glm::vec3 pos, float intesity,
		glm::vec3 color = glm::vec3(1, 1, 1));
	
	/*
	* Creates a bounding box that acts as a physics bound alongside with a VAO so it can be drawn
	*/
	entt::entity CreateBoundingBox(glm::vec3 min, glm::vec3 max, GLuint prograID = -1);

	/*
	* Inserts the registered entity to sceneObjects map with given name or given name + number returns identifier name
	*/
	inline std::string InsertSceneObject(std::string name, entt::entity entity)
	{
		auto it = sceneObjects.find(name);
		if (it != sceneObjects.end())
		{
			int i = 1;
			while (true)
			{
				it = sceneObjects.find(name + std::to_string(i));
				if (it == sceneObjects.end())
				{
					sceneObjects[name + std::to_string(i)] = entity;
					return name + std::to_string(i);
				}
				i++;
			}
		}
		else
			sceneObjects[name] = entity;
		return name;
	}
	/*
	* Gets the entity with the specified name
	*/
	inline entt::entity GetSceneObject(std::string name)
	{
		return sceneObjects.find(name) == sceneObjects.end() ? entt::tombstone : sceneObjects[name];
	}

	/*
	* Removes the enitity from the sceneObjects list and registry
	*/
	inline bool RemoveSceneObject(std::string name)
	{
		auto entity = sceneObjects[name];
		sceneObjects.erase(name);
		registry.destroy(entity);
		return entity != entt::tombstone;
	}

	inline std::map<std::string, entt::entity>::iterator sceneObjectsBegin()
	{
		return sceneObjects.begin(); 
	}

	inline std::map<std::string, entt::entity>::iterator sceneObjectsEnd()
	{
		return sceneObjects.end();
	}

	Camera camera;
	entt::registry registry;
	bool explicit_euler = true;
private:
	std::map<std::string, entt::entity> sceneObjects;
};