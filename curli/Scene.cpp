#include <Scene.h>

void CTransform::Update()
{
}

void CTriMesh::Update()
{
}

Scene::Scene()
{
}

Scene::~Scene()
{
}

void CLight::Update()
{
}

entt::entity Scene::CreateEntity()
{
	auto entity = registry.create();
	return entity;
}

void Scene::DestroyEntity(entt::entity entity)
{
	registry.destroy(entity);
}

void Scene::Update()
{
	//call update functions of every component
	registry.view<CRigidBody, CTransform>().each([](CRigidBody& rigidBody, CTransform& transform)
		{ 
			rigidBody.Update();
			transform.SetPivot(rigidBody.position);
			//transform.SetEulerRotation(rigidBody.rotation);
			transform.Update();
		});
	//registry.view<CTransform>().each([](CTransform& transform) { transform.Update(); });
	registry.view<CTriMesh>().each([](CTriMesh& mesh) { mesh.Update(); });
	registry.view<CLight>().each([](CLight& light) { light.Update(); });
}

entt::entity Scene::AddPointLight(glm::vec3 pos, float intesity, glm::vec3 color)
{
	auto entity = CreateEntity();
	registry.emplace<CLight>(entity, LightType::POINT, color, glm::min(intesity, 1.0f), pos, 
		glm::vec3(0, 0, 0), 0, 0);
	return entity;
}

entt::entity Scene::CreateModelObject(const std::string& meshPath, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateEntity();
	registry.emplace<CTriMesh>(entity, meshPath);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	registry.emplace<CPhongMaterial>(entity);
	return entity;
}

entt::entity Scene::CreateModelObject(cy::TriMesh& mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateEntity();
	registry.emplace<CTriMesh>(entity, mesh);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	return entity;
}

void CRigidBody::ApplyForce(glm::vec3 force)
{
	velocity += force / mass;
}

void CRigidBody::Update()
{
	/*float dragForceMagnitude = glm::pow(glm::length(velocity), 2.0f) * drag;
	glm::vec3 dragForceVector = dragForceMagnitude * -glm::normalize(velocity);
	printf("dragForceVector: %f, %f, %f\n velocity: %f, %f, %f, drag\n", dragForceVector.x, dragForceVector.y, dragForceVector.z, velocity.x, velocity.y, velocity.z, drag);
	if (glm::pow(glm::length(velocity), 2.0f) > 0.0f)
	{
		ApplyForce(dragForceVector);
		printf("hereaaaaaaaaaaaaaa\n");
	}
	else
	{
		velocity = glm::vec3(0);
		printf("here\n");
	}*/
	
	velocity *= 0.999f;


	position += velocity;
}

void CPhongMaterial::Update(){}
