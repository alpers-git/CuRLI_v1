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
		/*return glm::vec3(glm::degrees(atan2(eye.x - center.x, eye.z - center.z)),
			glm::degrees(atan2(eye.y - center.y, glm::length(glm::vec2(eye.x - center.x, eye.z - center.z)))),
			0.f); */
		return angles;
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
		
		this->angles = angles;
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
		printf("eye: %f %f %f\n", eye.x, eye.y, eye.z);
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

//struct OrbitCamera : Camera
//{
//public:
//	OrbitCamera(glm::vec3 center = glm::vec3(0.f,0.f,0.f), glm::vec3 angles = glm::vec3(0.f, 0.f, 0.f), float distance = 10.f,
//		float fov = 45.f, float near = 0.1f, float far = 100.f, float aspectRatio = 1.f, bool perspective = true)
//		:center(center), angles(angles), distance(distance), Camera(fov, near, far, aspectRatio, perspective)
//	{
//		this->CalculateViewMatrix();
//		this->CalculateProjectionMatrix();
//	}
//	void inline SetCenter(glm::vec3 center, bool recalculate = false)
//	{
//		this->center = center;
//		if (recalculate)
//			CalculateViewMatrix();
//		viewDirty = !recalculate;
//	}
//	void inline SetAngles(glm::vec3 angles, bool recalculate = false)
//	{
//		this->angles = angles;
//		if (recalculate)
//			CalculateViewMatrix();
//		viewDirty = !recalculate;
//	}
//	void inline SetDistance(float distance, bool recalculate = false)
//	{
//		this->distance = distance;
//		if (recalculate)
//			CalculateViewMatrix();
//		viewDirty = !recalculate;
//	}
//	glm::vec3 GetCenter() { return center; }
//	glm::vec3 GetAngles() { return angles; }
//	float GetDistance() { return distance; }
//private:
//	glm::vec3 center;
//	glm::vec3 angles;
//	float distance;
//	
//	void inline CalculateViewMatrix()
//	{
//		auto unitSpherePos = glm::vec3(sin(glm::radians(angles.x)) * cos(glm::radians(angles.y)),
//			sin(glm::radians(angles.y)),
//			cos(glm::radians(angles.x)) * cos(glm::radians(angles.y)));
//		bool flipY = abs(fmodf(angles.y, 360)) > 90.f && abs(fmodf(angles.y, 360)) < 270.f;
//		viewMatrix = glm::lookAt( center + distance * unitSpherePos,
//			center,
//			//Adjust the up vector to be perpendicular to the camera's direction
//			glm::normalize(glm::cross(-unitSpherePos, {0, flipY ? -1:1 ,0})));
//		viewDirty = false;
//	}
//};

struct CTransform
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
		modelMatrix = glm::translate(glm::mat4(1.f), position);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));//TODO!!!
		modelMatrix = glm::scale(modelMatrix, scale);
		
	}
	
private:
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);

	glm::mat4 modelMatrix = glm::mat4(1.f);
	bool modelDirty = false;

};

struct CTriMesh
{
public:
	CTriMesh(cy::TriMesh& mesh)
		:mesh(mesh)
	{}
	cy::TriMesh& GetMesh() { return mesh; }
	
	unsigned int GetNumVertices() { return mesh.NV(); }
	unsigned int GetNumFaces() { return mesh.NF(); }
	glm::vec3 GetVertex(unsigned int index) { return glm::cy2GLM(mesh.V(index)); }
	glm::vec3 GetNormal(unsigned int index) { return glm::cy2GLM(mesh.VN(index)); }
	//glm::vec3 GetFaceNormal(unsigned int index) { return glm::cy2GLM(mesh.FN(index)); }

	void LoadObj(const std::string& path)
	{
		mesh.LoadFromFileObj(path.c_str());
	}
	
private:
	cy::TriMesh& mesh;
};

class Scene
{
public:
	Scene();
	~Scene();

	void AddEntity(entt::entity entity);
	void RemoveEntity(entt::entity entity);
	void AddComponent(entt::entity entity, entt::id_type component);
	void RemoveComponent(entt::entity entity, entt::id_type component);

	Camera camera;
private:
	entt::registry registry;
};