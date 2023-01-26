#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Camera
{
private:

	glm::vec3 eye = glm::vec3(1.f);
	glm::vec3 center = glm::vec3(0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	float fov = 45.f;
	float aspectRatio = 1.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	bool perspective = true;

	bool viewDirty = false;
	bool projectionDirty = false;

	glm::mat4 viewMatrix = glm::lookAt(eye, center, up);
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);


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

public:
	Camera()
	{}
	Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up,
		float fov = 45.f, float near = 0.1f, float far = 100.f, float aspectRatio = 1.f, bool perspective = true)
		:eye(eye), center(center), up(up), fov(fov), nearPlane(near), farPlane(far), aspectRatio(aspectRatio), perspective(perspective)
	{
		viewMatrix = glm::lookAt(eye, center, up);
		if (perspective)
			projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, near, far);
		else
			projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, near, far);
	}
	void SetPerspective(bool perspective, bool recalculate = false)
	{
		this->perspective = perspective;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void SetFOV(float fov, bool recalculate = false)
	{
		this->fov = fov;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void SetAspectRatio(float aspectRatio, bool recalculate = false)
	{
		this->aspectRatio = aspectRatio;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void SetNearPlane(float nearPlane, bool recalculate = false)
	{
		this->nearPlane = nearPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void SetFarPlane(float farPlane, bool recalculate = false)
	{
		this->farPlane = farPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void SetEye(glm::vec3 eye, bool recalculate = false)
	{
		this->eye = eye;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void SetCenter(glm::vec3 center, bool recalculate = false)
	{
		this->center = center;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void SetUp(glm::vec3 up, bool recalculate = false)
	{
		this->up = up;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	glm::vec3 GetEye() { return eye; }
	glm::vec3 GetCenter() { return center; }
	glm::vec3 GetUp() { return up; }
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

};

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

class Scene
{
public:
	Scene();
	~Scene();

private:
	entt::registry registry;
};