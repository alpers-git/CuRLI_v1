#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cyTriMesh.h>
#include <CyToGLMHelper.h>
#include <glm/gtx/log_base.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <stdarg.h>
#include <string>
#include <unordered_map>
#include <lodepng.h>

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

enum class CType{
	Transform, TriMesh, 
	PhongMaterial, ImageMap,
	Light,
	BoundingBox,VelocityField2D,
	ForceField2D, RigidBody,
	Count
};
struct Component
{
	virtual void Update() = 0;
	CType GetType() { return this->type; }
protected:
	CType type;
	
};

struct CTransform : Component
{
public:
	CTransform(glm::vec3 position = glm::vec3(0.f), glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f))
		: position(position), rotation(rotation), scale(scale)
	{
		//pivot = position;
		CalculateModelMatrix();
		type = CType::Transform;
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

enum class ShadingMode
{
	PHONG, EDITOR
};

struct CTriMesh : Component
{
public:
	static constexpr CType type = CType::TriMesh;
	CTriMesh(cy::TriMesh& mesh)
	{
		InitializeFrom(mesh);
	}

	CTriMesh(const std::string& path)
	{
		LoadObj(path.c_str());
	}
	
	CTriMesh()
	{
	}
	
	//cy::TriMesh& GetMesh() { return mesh; }
	
	unsigned int GetNumVertices() { return vertices.size(); }
	unsigned int GetNumFaces() { return faces.size(); }
	unsigned int GetNumNormals() { return vertexNormals.size(); }
	unsigned int GetNumTextureVertices() { return  textureCoords.size(); }
	
	glm::vec3 GetVertex(unsigned int index) { return vertices.at(index); }
	glm::vec3 GetVNormal(unsigned int index) { return vertexNormals.at(index);}
	glm::vec2 GetVTexture(unsigned int index) { return textureCoords.at(index); }
	glm::uvec3 GetFace(unsigned int index) { return faces.at(index); }
	
	
	//unsigned int GetNumMaterials() { return mesh.NM(); }
	
	/*glm::vec3 GetMatAmbientColor(unsigned int index) { return glm::make_vec3(mesh.M(index).Ka); }
	glm::vec3 GetMatDiffuseColor(unsigned int index) { return glm::make_vec3(mesh.M(index).Kd); }
	glm::vec3 GetMatSpecularColor(unsigned int index) { return glm::make_vec3(mesh.M(index).Ks); }*/
	
	/*std::string GetMatAmbientTexture(unsigned int index) 
	{ 
		if (GetNumMaterials() > index && mesh.M(index).map_Ka.data == nullptr)
			return std::string();
		else
			return mesh.M(index).map_Ka; 
	}
	std::string GetMatDiffuseTexture(unsigned int index) 
	{
		if (GetNumMaterials() > index && mesh.M(index).map_Kd.data == nullptr)
			return std::string();
		else
			return mesh.M(index).map_Kd;
	}
	std::string GetMatSpecularTexture(unsigned int index)
	{
		if (GetNumMaterials() > index && mesh.M(index).map_Ks.data == nullptr)
			return std::string();
		else
			return mesh.M(index).map_Ks;
	}*/
	
	/*glm::ivec3 GetFNormal(unsigned int index) { 
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
	}*/

	void* GetVertexDataPtr() { return &vertices.front(); }
	void* GetNormalDataPtr() { return &vertexNormals.front(); }
	void* GetTextureDataPtr() { return &textureCoords.front();}
	void* GetFaceDataPtr() { return &faces.front(); }

	
	/*
	* Create opengl friendly single buffer indexed mesh from cy::TriMesh
	*/
	void InitializeFrom(cy::TriMesh& mesh);

	inline void ComputeBoundingBox()
	{
		for (auto vertex : vertices)
		{
			bBoxMax = glm::max(bBoxMax, vertex);
			bBoxMin = glm::min(bBoxMin, vertex);
		}
		bBoxInitialized = true;
	}

	inline void ComputeNormals()
	{
		vertexNormals.resize(vertices.size());
		for(auto face : faces)
		{
			glm::vec3 v1 = vertices[face.x];
			glm::vec3 v2 = vertices[face.y];
			glm::vec3 v3 = vertices[face.z];

			glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

			vertexNormals[face.x] += normal;
			vertexNormals[face.y] += normal;
			vertexNormals[face.z] += normal;
		}
		
		for (auto& normal : vertexNormals)
			normal = glm::normalize(normal);
	}
	
	inline glm::vec3 GetBoundingBoxMin()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return bBoxMin;
	}

	inline glm::vec3 GetBoundingBoxMax()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return bBoxMax;
	}

	inline glm::vec3 GetBoundingBoxCenter()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return (bBoxMin + bBoxMax) * .5f;
	}

	inline void LoadObj(const std::string& path)
	{
		cy::TriMesh mesh;
		mesh.LoadFromFileObj(path.c_str());
		InitializeFrom(mesh);
		
		printf("Loaded model with %d vertices, vertex normals %d, texture vertices %d, and faces %d from %s\n",
			GetNumVertices(),
			GetNumNormals(),
			GetNumTextureVertices(),
			GetNumFaces(),
			path.c_str());
	}
	
	inline ShadingMode GetShadingMode() { return shading; }
	inline void SetShadingMode(ShadingMode mode) { shading = mode; }

	void Update();
	
	void Clear()
	{
		bBoxInitialized = false;
		vertices.clear();
		vertexNormals.clear();
		textureCoords.clear();
		faces.clear();
	}

	
	bool visible = true;
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::uvec3> faces;
	
	glm::vec3 bBoxMin = glm::vec3(FLT_MAX);
	glm::vec3 bBoxMax = glm::vec3(-FLT_MAX);
	bool bBoxInitialized = false;
	ShadingMode shading = ShadingMode::PHONG; //0 = phong-color, 1 = phong-texture, 2 = editor mode
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
	static constexpr CType type = CType::Light;
	CLight(LightType ltype, glm::vec3 color, float intensity, glm::vec3 position,
		glm::vec3 direction, float iCutoff, float oCutoff)
		:lightType(ltype)
	{
		this->color = color;
		this->intensity = intensity;
		//Point light constructor
		if (ltype == LightType::POINT)
		{
			this->position = position;
			//invalid values for safety
			direction = glm::vec3(NAN, NAN, NAN);
			innerCutOff = -1;
			outerCutoff = -1;
		}
		//Directional light constructor
		else if (ltype == LightType::DIRECTIONAL)
		{
			this->direction = direction;
			lightType = LightType::DIRECTIONAL;

			//invalid values for safety
			position = glm::vec3(NAN, NAN, NAN);
			innerCutOff = -1;
			outerCutoff = -1;
		}
		//Spot light constructor
		else if (ltype == LightType::SPOT)
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
	static constexpr CType type = CType::RigidBody;
	CRigidBody(float mass = 1.0f, glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 rotation= glm::vec3(0.0f), float drag = 0.0f)
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
	static constexpr CType type = CType::PhongMaterial;
	glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.3f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(0.9f, 0.9f, 0.9f);
	float shininess = 450.f;

	void Update();
};

struct ImageMap
{
	enum class BindingSlot{
		T_AMBIENT, T_DIFFUSE, T_SPECULAR, NORMAL, BUMP
	};
public:
	ImageMap(std::string path, BindingSlot slot)
		:path(path), bindingSlot(slot)
	{
		std::vector<unsigned char> png;
		//load and decode
		unsigned error = lodepng::load_file(png, path);
		if (error)
		{
			printf("Texture constructor\n\tlodepng:load error %d - %s\n", error, lodepng_error_text(error));
			return;
		}
		error = lodepng::decode(image, (dims.x), (dims.y), png);

		//if there's an error, display it
		if (error)
		{
			printf("Texture constructor\n\tlodepng:decoder error %d - %s\n", error, lodepng_error_text(error));
			return;
		}
	}
	//getters
	std::string GetPath() { return path; }
	std::vector<unsigned char> GetImage() { return image; }
	glm::uvec2 GetDims() { return dims; }
	BindingSlot GetBindingSlot() { return bindingSlot; }
	std::string GetSlotName();

	GLuint glID; //Maybe find a better way...
private:
	std::string path;
	std::vector<unsigned char> image;
	glm::uvec2 dims;
	BindingSlot bindingSlot;
};
struct CImageMaps : Component// TODO: Move to OpenGLProgram.h, instead have a component that list texture data and what uniform it binds to...
{
public:
	static constexpr CType type = CType::ImageMap;
	
	CImageMaps()
	{}
	
	void AddImageMap(ImageMap::BindingSlot slot, std::string path);
	void RemoveMap(ImageMap::BindingSlot slot);
	void Update();

	unsigned int inline GetNumMaps() { return imgMaps.size(); }
	std::unordered_map<ImageMap::BindingSlot, ImageMap>::iterator mapsBegin() { return imgMaps.begin(); }
	std::unordered_map<ImageMap::BindingSlot, ImageMap>::iterator mapsEnd() { return imgMaps.end(); }

	bool dirty = false;
private:
	std::unordered_map<ImageMap::BindingSlot,ImageMap> imgMaps;
};

enum class FieldPlane
{
	XY, YZ, XZ
};
struct CVelocityField2D : Component
{
public:
	static constexpr CType type = CType::VelocityField2D;
	void Update();
	//function pointer here
	std::function<glm::vec2 (glm::vec2)> field;
	float scaling = 1.f;
	
	FieldPlane plane;
	
	CVelocityField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{
	}

	glm::vec3 VelocityAt(glm::vec2 p);
};

struct CForceField2D : Component
{
public:
	static constexpr CType type = CType::ForceField2D;
	void Update();
	//function pointer here
	std::function<glm::vec2(glm::vec2)> field;
	float scaling = 1.f;
	FieldPlane plane;

	CForceField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{
	}

	glm::vec3 ForceAt(glm::vec2 p);
};

struct CBoundingBox : Component
{
public:
	friend class Scene;
	static constexpr CType type = CType::BoundingBox;
	CBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		this->min = glm::min(min, max);
		this->max = glm::max(min, max);
	}
	
	glm::vec3 GetMin() { return min; }
	glm::vec3 GetMax() { return max; }
	
	void SetMin(glm::vec3 min) { this->min = min; dirty = true; }
	void SetMax(glm::vec3 max) { this->max = max; dirty = true; }
	
	inline bool IsDirty() { return dirty; }


	void Update();
	void Rebound(CRigidBody& rigidBody);
private:
	glm::vec3 min;
	glm::vec3 max;
	bool dirty = false;
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

	inline std::unordered_map<std::string, entt::entity>::iterator sceneObjectsBegin()
	{
		return sceneObjects.begin(); 
	}

	inline std::unordered_map<std::string, entt::entity>::iterator sceneObjectsEnd()
	{
		return sceneObjects.end();
	}
	
	/*
	* Returns true if the entity has the given component
	*/
	bool EntityHas(entt::entity e, CType component);

	Camera camera;
	entt::registry registry;
	bool explicit_euler = true;
private:
	std::unordered_map<std::string, entt::entity> sceneObjects;
};