#pragma once
#include <Scene.h>
#include <GLFWHandler.h>
#include <glm/gtx/component_wise.hpp>
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
		float deltaTime = (GLFWHandler::GetInstance().GetTime() - t)*10.0f;//Time between two frames scaled up 10 times

		for (float step = 0; step < deltaTime; step += tStepSize)
		{
			//Synchronize the physics objects before integration
			static_cast<T*>(this)->Synchronize();
			static_cast<T*>(this)->Integrate(tStepSize);
		}

		t = GLFWHandler::GetInstance().GetTime();
	}

	/*
	* Prepares the physics objects before integration
	*/
	void Synchronize() 
	{
		scene->registry.view<CRigidBody, CTransform>()
		.each([&](CRigidBody& rigidBody, CTransform& transform)
		{
			transform.SetPosition(rigidBody.position);
			transform.SetEulerRotation(rigidBody.GetOrientationMatrix());
			transform.Update();
		});
		
		scene->registry.view<CBoxCollider, CTransform, CTriMesh>()
		.each([&](CBoxCollider& collider, CTransform& transform, CTriMesh& mesh)
		{
			collider.SetBounds(transform.GetModelMatrix() * glm::vec4(mesh.GetBoundingBoxMin(), 1.0f),
			transform.GetModelMatrix() * glm::vec4(mesh.GetBoundingBoxMax(), 1.0f));
		});
	};
	
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
	const float tStepSize = 0.001f;
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
					rb.ApplyLinearImpulse(fFromField * dt);

				rb.TakeFwEulerStep(dt);

				//check for collisions with world boundary
				auto* boxCollider = scene->registry.try_get<CBoxCollider>(entity);
				if (boxCollider)
				{
					entt::entity e = scene->registry.view<CPhysicsBounds>().front();
					//check if null entity
					if (e != entt::null)
					{
						auto* bounds = scene->registry.try_get<CPhysicsBounds>(e);
						if (bounds)
						{
							const glm::vec3 min = bounds->GetMin();
							const glm::vec3 max = bounds->GetMax();
							
							//Go over the vertices of the box collider and figure if any of them are outside the bounds
							//if there is a collision init the collision normal to point inward to the bounds
							glm::vec3 collisionNormal = glm::vec3(0.0f);
							glm::vec3 collisionVert = glm::vec3(0.0f);
							int numCollisions = 0;
							glm::vec3 collidedBoundFace = glm::vec3(0.0f);
							for (int i = 0; i < 8; i++)
							{
								const glm::vec3 v = boxCollider->vertices[i];
								if (v.x < min.x)
								{
									collisionNormal.x += 1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(min.x, 0.0f, 0.0f);
									//break;
								}
								else if (v.x > max.x)
								{
									collisionNormal.x += -1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(max.x, 0.0f, 0.0f);
									//break;
								}
								if (v.y < min.y)
								{
									collisionNormal.y += 1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(0, min.y, 0);
									//break;
								}
								else if (v.y > max.y)
								{
									collisionNormal.y += -1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(0, max.y, 0);
									//break;
								}
								if (v.z < min.z)
								{
									collisionNormal.z += 1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(0, 0, min.z);
									//break;
								}
								else if (v.z > max.z)
								{
									collisionNormal.z += -1.0f;
									collisionVert += v;
									numCollisions++;
									collidedBoundFace = glm::vec3(0, 0, max.z);
									//break;
								}
							}
							if (glm::length(collisionNormal) > 0.0f) //Resolve the collision
							{
								//Check if object has sufficient momentum to bounce of the object
								if (glm::abs(glm::dot(collisionNormal, rb.linearMomentum)) < 0.001f )
								{
									//if not the object is residing the boundary dont test for collision
									rb.linearMomentum -= collisionNormal * rb.linearMomentum;
									return;
								}

								collisionVert = collisionVert /((float) numCollisions);//Avg box collision

								//For more accurate collisions find the closest vertex on model
								const glm::mat4 modelMat = scene->registry.get<CTransform>(entity).GetModelMatrix();
								const glm::vec3 modelSpaceCollisionVert = 
									glm::inverse(modelMat) * glm::vec4(collisionVert,1.f);
								const glm::vec3 accurateCollisionVert = modelMat * glm::vec4(scene->registry.get<CTriMesh>(entity).
									ApproximateTheClosestPointTo(collisionVert, 10), 1.0f);

								collisionNormal = glm::normalize(collisionNormal);
								const glm::vec3 r = accurateCollisionVert - rb.position;
								float impulseMag = bounds->MagImpulseCollistionFrom(
									boxCollider->elasticity, rb.mass,
									rb.GetInertiaTensor(), rb.GetVelocity(),
									collisionNormal, r);
								
								const float penetration = -glm::dot(collisionVert - collidedBoundFace, collisionNormal);
								rb.ApplyLinearImpulse(collisionNormal * impulseMag);

								rb.ApplyAngularImpulse(impulseMag * glm::cross(r, collisionNormal) * 100.f);
								rb.position += penetration * collisionNormal * 2.5f;//Apply perturbation
							}
						}
					}	
				}				
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