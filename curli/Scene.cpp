#include <Scene.h>
#include <GLFWHandler.h>

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

glm::vec3 CVelocityField2D::VelocityOn(glm::vec2 p)
{
	switch (this->plane)
	{
	case FieldPlane::XY:
		return scaling * glm::vec3(field(p), 0.0f);
		break;
	case FieldPlane::XZ:
	{
		auto t = field(p);
		return scaling * glm::vec3(t.x, 0.0f, t.y);
		break;
	}
	case FieldPlane::YZ:
	{
		auto t = field(p);
		return scaling * glm::vec3(0.0f, field(p).x, field(p).y);
		break;
	}
	default:
		return glm::vec3(0.0f);
		break;
	}
}

void CVelocityField2D::Update()
{
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
	velocity += acceleration;

	velocity *= 0.999f;

	position += velocity;



}

void CPhongMaterial::Update() 
{}

entt::entity Scene::CreateSceneObject(std::string name)
{
	auto entity = registry.create();
	InsertSceneObject(name, entity);
	return entity;
}

//void Scene::DestroyEntity(entt::entity entity)
//{
//	registry.destroy(entity);
//}

void Scene::Update()
{
	//call update functions of every component
	registry.view<CRigidBody, CTransform>().each([&](CRigidBody& rigidBody, CTransform& transform)
		{
			registry.view<CVelocityField2D>().each([&](auto vField)
				{
					rigidBody.velocity = vField.VelocityOn(
						glm::vec2(rigidBody.position.x, rigidBody.position.z));
				});
			rigidBody.Update();
			transform.SetPosition(rigidBody.position);
			//transform.SetEulerRotation(rigidBody.rotation);
			transform.Update();
		});
	//registry.view<CTransform>().each([](CTransform& transform) { transform.Update(); });
	registry.view<CTriMesh>().each([&](CTriMesh& mesh) { mesh.Update(); });
	registry.view<CLight>().each([&](CLight& light) { light.Update(); });
	
}

entt::entity Scene::AddPointLight(glm::vec3 pos, float intesity, glm::vec3 color)
{
	auto entity = CreateSceneObject("light");
	registry.emplace<CLight>(entity, LightType::POINT, color, glm::min(intesity, 1.0f), pos, 
		glm::vec3(0, 0, 0), 0, 0);
	return entity;
}

entt::entity Scene::CreateModelObject(const std::string& meshPath, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	//from path get the part after the last "/" and before "."
	auto name = meshPath.substr(meshPath.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of("."));
	auto entity = CreateSceneObject(name);
	
	registry.emplace<CTriMesh>(entity, meshPath);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	registry.emplace<CPhongMaterial>(entity);
	
	
	//InsertSceneObject(name, entity);

	return entity;
}

entt::entity Scene::CreateModelObject(cy::TriMesh& mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateSceneObject("unnamed-");
	registry.emplace<CTriMesh>(entity, mesh);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	registry.emplace<CPhongMaterial>(entity);
	//InsertSceneObject("unnamed-", entity);
	return entity;
}

void CRigidBody::ApplyForce(glm::vec3 force)
{
	velocity += force / mass;
}
