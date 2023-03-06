#pragma once
#include <Scene.h>
#include <GLFWHandler.h>
#include <ApplicationState.h>

template <typename T>
class PhysicsIntegrator
{
public:
	PhysicsIntegrator(std::shared_ptr<Scene> scene) : scene(scene) 
	{ 
		//t = GLFWHandler::GetInstance().GetTime();
	}
	~PhysicsIntegrator() {}

	void Update()
	{
		float deltaTime = GLFWHandler::GetInstance().GetTime() - t;//Time between two frames

		for (float step = 0; step < deltaTime; step += tStepSize)
		{
			static_cast<T*>(this)->Integrate(tStepSize);
		}

		t = GLFWHandler::GetInstance().GetTime();
	}
	
	/*
	* Integrates the scene forward in time by dt
	*/
	virtual void Integrate(float dt) = 0;
	
	/*
	* Called by DispatchEvent when mouse buttons are used
	*/
	void OnMouseButton(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
			m1Down = true;
		else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
			m1Down = false;
		else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
			m2Down = false;
	};
	/*
	* Called by DispatchEvent when mouse is moved
	*/
	void OnMouseMove(double x, double y) {
		bool updateMousePos = true;
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		entt::entity curEntity = ApplicationState::GetInstance().selectedObject;
		
		auto* rb = scene->registry.try_get<CRigidBody>(curEntity);
		auto* ms = scene->registry.try_get<CTriMesh>(curEntity);
		auto* tr = scene->registry.try_get<CTransform>(curEntity);
		
		if (rb && ms && tr)
		{
			Camera cam = scene->camera;
			glm::vec3 forward = glm::normalize(cam.GetCenter() - cam.GetLookAtEye());
			glm::vec3 right = glm::normalize(glm::cross(forward, {0,1,0}));
			glm::vec3 up = glm::normalize(glm::cross(forward, -right));
			if (m1Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				rb->ApplyLinearImpulse((-deltaPos.x * right + deltaPos.y * up) * 0.1f);
			}
			if (m2Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				rb->ApplyAngularImpulse((deltaPos.x * -up + deltaPos.y * -right) * 800.1f);
			}
		}

		if (updateMousePos)
			prevMousePos = { x,y };
			
	};
	/*
	* Called by DispatchEvent when keyboard is used
	*/
	void OnKeyboard(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
			shiftDown = true;
		else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
			shiftDown = false;

		ApplicationState::GetInstance().physicsInteraction = shiftDown;
	};
	
protected:
	glm::vec3& AccumilateVelocityFields(glm::vec3 pos)
	{
		glm::vec3 vFromField = glm::vec3(0.0f);
		scene->registry.view<CVelocityField2D>()
			.each([&vFromField, pos](auto fieldEntity, CVelocityField2D& field)
				{
					vFromField += field.At(pos);
				});
		return vFromField;
	}
	glm::vec3& AccumilateForceFields(glm::vec3 pos)
	{
		glm::vec3 fFromField = glm::vec3(0.0f);
		scene->registry.view<CForceField2D>()
			.each([&fFromField, pos](auto fieldEntity, CForceField2D& field)
				{
					fFromField += field.At(pos);
				});
		return fFromField;
	}
	
	std::shared_ptr<Scene> scene;
	const float tStepSize = 0.0001f;
	float t = 0.0f;
	
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;

	bool shiftDown = false;
};

class EmptyIntegrator : public PhysicsIntegrator<EmptyIntegrator>
{
public:
	EmptyIntegrator(std::shared_ptr<Scene> scene) : PhysicsIntegrator(scene) {}
	~EmptyIntegrator() {}

	void Integrate(float dt) override
	{}
};

class FwEulerIntegrator : public PhysicsIntegrator<FwEulerIntegrator>
{
public:
	FwEulerIntegrator(std::shared_ptr<Scene> scene) : PhysicsIntegrator(scene) {}
	~FwEulerIntegrator() {}

	void Integrate(float dt) override
	{
		scene->registry.view<CRigidBody>()
		.each([dt, this](auto entity, CRigidBody& rb)
		{
			glm::vec3 vFromField = AccumilateVelocityFields(rb.position);
			if (glm::length(vFromField) > 0.0f)
			{
				//rb.velocity = vFromField;
				rb.position += vFromField * dt;
			}
			else 
			{
				glm::vec3 fFromField = AccumilateForceFields(rb.position);
				if (rb.mass > 0.0000001f)
					rb.ApplyLinearImpulse(fFromField*dt);
				/*glm::vec3 v = rb.GetVelocity();
				rb.ApplyLinearImpulse(-0.5f * rb.drag * v * v * dt);
				rb.position += v * dt;

				glm::vec3 w = rb.GetAngularVelocity();
				rb.ApplyAngularImpulse(-0.5f * rb.drag * w  * w * dt);
				rb.rotation += w * dt;*/
				rb.TakeFwEulerStep(dt);
			}
		});
	}
};

class BwEulerIntegrator : public PhysicsIntegrator<BwEulerIntegrator>
{
	public:
		BwEulerIntegrator(std::shared_ptr<Scene> scene) : PhysicsIntegrator(scene) {}
		~BwEulerIntegrator() {}

		void Integrate(float dt) override
		{
			
			scene->registry.view<CRigidBody>()
			.each([dt, this](auto entity, CRigidBody& rb)
			{
				////explicit step
				//glm::vec3 vFromField = glm::vec3(0.0f);
				//scene->registry.view<CVelocityField2D>()
				//.each([&vFromField, entity, rb](auto fieldEntity, CVelocityField2D& field)
				//	{
				//		vFromField += field.At(rb.position);
				//	});
				//if (glm::length(vFromField) > 0.0f)
				//{
				//	rb.velocity = vFromField;
				//	rb.position += rb.velocity * dt;
				//}
				//else
				//{
				//	glm::vec3 fFromField = glm::vec3(0.0f);
				//	scene->registry.view<CForceField2D>()
				//	.each([&fFromField, entity, rb](auto fieldEntity, CForceField2D& field)
				//		{
				//			fFromField += field.At(rb.position);
				//		});
				//	glm::vec3 acceleration = rb.acceleration;
				//	glm::vec3 velocity = rb.velocity;
				//	glm::vec3 position = rb.position;
				//	if (rb.mass > 0.00001f)
				//		acceleration += fFromField / rb.mass * dt;
				//	acceleration -= 0.5f * rb.drag * velocity * velocity;
				//	velocity += acceleration * dt;
				//	position += velocity * dt;

				//	//implcit step using explicit pos
				//	fFromField = glm::vec3(0.0f);
				//	scene->registry.view<CForceField2D>()
				//	.each([&fFromField, entity, position](auto fieldEntity, CForceField2D& field)
				//		{
				//			fFromField += field.At(position);
				//		});
				//	if (rb.mass > 0.00001f)
				//		rb.acceleration += fFromField / rb.mass * dt;
				//	rb.acceleration -= 0.5f * rb.drag * velocity * velocity;
				//	rb.velocity += rb.acceleration * dt;
				//	rb.position += rb.velocity * dt;
				//}
			});
		}
};