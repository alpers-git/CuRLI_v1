#pragma once
#include <Scene.h>
#include <GLFWHandler.h>

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
	const float tStepSize = 0.00001f;
	float t = 0.0f;
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
					rb.velocity = vFromField;
					rb.position += rb.velocity * dt;
				}
				else 
				{
					glm::vec3 fFromField = AccumilateForceFields(rb.position);
					if (rb.mass > 0.0000001f)
						rb.ApplyLinearImpulse(fFromField*dt);
					rb.ApplyLinearImpulse(-0.5f * rb.drag * rb.GetVelocity() * rb.GetVelocity() * dt);
					rb.position += rb.GetVelocity() * dt;

					//rb.ApplyAngularImpulse(glm::vec3(100.2f, 0, 0) * dt);
					rb.ApplyAngularImpulse(-0.5f * rb.drag * rb.GetAngularVelocity() * rb.GetAngularVelocity() * dt);
					rb.rotation += rb.GetAngularVelocity() * dt;
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