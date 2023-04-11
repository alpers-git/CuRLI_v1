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

		scene->registry.view<CSoftBody>()
			.each([&](const auto entity, CSoftBody sb)
				{
					//Create event 
					Event event;
					event.type = Event::Type::SoftbodySim;
					event.softbodySim.e = entity;
					GLFWHandler::GetInstance().QueueEvent(event);
				});

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
			//transform.Update();
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
		auto* sb = scene->registry.try_get<CSoftBody>(curEntity);
		auto* ms = scene->registry.try_get<CTriMesh>(curEntity);
		auto* tr = scene->registry.try_get<CTransform>(curEntity);
		
		Camera cam = scene->camera;
		glm::vec3 forward = glm::normalize(cam.GetCenter() - cam.GetLookAtEye());
		glm::vec3 right = glm::normalize(glm::cross(forward, {0,1,0}));
		glm::vec3 up = glm::normalize(glm::cross(forward, -right));
		if (rb && ms && tr)
		{
			if (m1Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				rb->ApplyLinearImpulse((-deltaPos.x * right + deltaPos.y * up) * 0.1f);
			}
			if (m2Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				rb->ApplyAngularImpulse((deltaPos.x * -up + deltaPos.y * -right) * 80.1f);
			}
		}
		else if (sb && ms && tr)
		{
			if (m1Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				//sb->ApplyLinearImpulse((-deltaPos.x * right + deltaPos.y * up) * 0.1f);
				//random number between 0 and numOfNodes in the selected softbody
				int randomNode = rand() % sb->nodes.size();
				sb->nodes[randomNode].acceleration += (-deltaPos.x * right + deltaPos.y * up) * 0.1f;
			}
			if (m2Down && shiftDown)
			{
				ApplicationState::GetInstance().physicsInteraction = true;
				//sb->ApplyAngularImpulse((deltaPos.x * -up + deltaPos.y * -right) * 80.1f);
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
							glm::vec3 collisionNormal(0.0f);
							glm::vec3 collisionVert(0.0f);
							float penetration = 0;
							if ( bounds->IsCollidingWith(*boxCollider, collisionNormal, collisionVert, penetration) ) //Resolve the collision
							{
								//Check if object has sufficient momentum to bounce of the object
								if (glm::abs(glm::dot(collisionNormal, rb.linearMomentum)) < 0.001f )
								{
									//if not the object is residing the boundary dont test for collision
									rb.linearMomentum -= collisionNormal * rb.linearMomentum;
									//rb.angularMomentum = glm::vec3(0);
									//return;
								}

								//For more accurate collisions find the closest vertex on model
								const glm::mat4 modelMat = scene->registry.get<CTransform>(entity).GetModelMatrix();
								const glm::vec3 modelSpaceCollisionVert = 
									glm::inverse(modelMat) * glm::vec4(collisionVert,1.f);
								const glm::vec3 accurateCollisionVert = modelMat * glm::vec4(scene->registry.get<CTriMesh>(entity).
									ApproximateTheClosestPointTo(modelSpaceCollisionVert, max(1, scene->registry.get<CTriMesh>(entity).GetNumVertices()/10)), 1.0f);

								const glm::vec3 r = collisionVert - rb.position;
								float impulseMag = bounds->MagImpulseCollistionFrom(
									boxCollider->elasticity, rb.mass,
									rb.GetInertiaTensor(), rb.GetVelocity(),
									collisionNormal, r);
								
								rb.ApplyLinearImpulse(collisionNormal * impulseMag);
								const auto angImp = impulseMag * glm::cross(r, collisionNormal) *1.f;
								rb.ApplyAngularImpulse(angImp);
								int colCounter = 0;
								do
								{
									rb.position -= penetration * collisionNormal;//Apply perturbation until no longer colliding
									rb.linearMomentum -= penetration * collisionNormal;
									this->Synchronize();
									if (colCounter++ > 5)
										break;
									
								} while (bounds->IsCollidingWith(*boxCollider, collisionNormal, collisionVert, penetration));
							}
						}
					}	
				}				
			}
		});

		scene->registry.view<CSoftBody>()
		.each([dt, this](auto entity, CSoftBody& sb)
		{
			sb.TakeFwEulerStep(dt);
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