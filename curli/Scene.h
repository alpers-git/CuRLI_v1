#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <cyTriMesh.h>
#include <CyToGLMHelper.h>
#include <glm/gtx/log_base.hpp>
#include <glm/gtc/matrix_access.hpp>

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
		eye = center + glm::normalize(eye - center) * distance;
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
	//getter and setters
	void SetPosition(glm::vec3 position, bool recalculate = false)
	{
		this->position = position;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetRotation(glm::vec3 rotation, bool recalculate = false)
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
	glm::vec3 GetPosition() { return position; }
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
		modelMatrix = glm::scale(glm::mat4(1.f), scale);
		modelMatrix = modelMatrix * glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
		modelMatrix = glm::translate(modelMatrix, position);

		modelDirty = false;
	}

	void Update();
	
private:
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);

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
	glm::vec3 GetVertex(unsigned int index) { return glm::cy2GLM(mesh.V(index)); }
	glm::vec3 GetNormal(unsigned int index) { return glm::cy2GLM(mesh.VN(index)); }
	//glm::vec3 GetFaceNormal(unsigned int index) { return glm::cy2GLM(mesh.FN(index)); }

	inline void ComputeBoundingBox()
	{
		mesh.ComputeBoundingBox();
		bBoxInitialized = true;
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
		printf("Loaded %d vertices and %d faces\n",
			GetNumVertices(),
			GetNumFaces());
	}

	void Update();
	
private:
	cy::TriMesh mesh;
	bool bBoxInitialized = false;
};

class Scene
{
public:
	Scene();
	~Scene();

	entt::entity CreateEntity();
	void DestroyEntity(entt::entity entity);
	void Update();
	
	template <typename C>
	void AddComponent(entt::entity& entity)
	{
		if (registry.all_of<C>(entity)) {
			registry.replace<C>(entity);
		}
		else {
			registry.emplace<C>(entity);
		}
	};
	
	template <typename C>
	void RemoveComponent(entt::entity& entity)
	{
		registry.remove<C>(entity);
	};
	
	/*
	* Returns a reference to the specified component of the specified entity. 
	* Use empty entity to fetch the first entity with the specified component
	*/
	template <typename C>
	C& GetComponent(entt::entity& entity)
	{
		return registry.view<C>().get<C>(entity);
	};


	Camera camera;
	entt::registry registry;
private:
};