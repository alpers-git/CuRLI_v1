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

glm::vec3 CVelocityField2D::VelocityAt(glm::vec2 p)
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
	const float stepSize = 1.f;
	//euler method
	/*velocity += acceleration * stepSize;

	velocity += glm::pow(glm::length(velocity + 0.0000001f), 2.0f) 
		* drag * -glm::normalize(velocity + 0.0000001f) * stepSize;

	position += velocity * stepSize;*/
		
	//midpoint method
	const glm::vec3 midVelocity = velocity + 0.5f * stepSize * acceleration
		+ glm::pow(glm::length(velocity + 0.0000001f), 2.0f)
		* drag * -glm::normalize(velocity + 0.0000001f) * 0.5f * stepSize;
	const glm::vec3 midPosition = position + 0.5f * stepSize * midVelocity;
	velocity += 0.5f * stepSize * acceleration
		+ glm::pow(glm::length(velocity + 0.0000001f), 2.0f)
		* drag * -glm::normalize(velocity + 0.0000001f) *  stepSize;
	position += 0.5f * stepSize * midVelocity;
}

void CForceField2D::Update()
{
}

glm::vec3 CForceField2D::ForceAt(glm::vec2 p)
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
			int i = 0;
			glm::vec3 velocityFContribution = glm::vec3(0.0f);
			registry.view<CVelocityField2D>().each([&](auto vField)
				{
					velocityFContribution += vField.VelocityAt(
						glm::vec2(rigidBody.position.x, rigidBody.position.z));
					i++;
				});
			if(i!=0)
				rigidBody.velocity = velocityFContribution;
			
			i = 0;
			glm::vec3 forceFContribution = glm::vec3(0.0f);
			registry.view<CForceField2D>().each([&](auto fField)
				{
					forceFContribution += fField.ForceAt(
						glm::vec2(rigidBody.position.x, rigidBody.position.z));
					i++;
				});
			if (i != 0)
				rigidBody.acceleration = forceFContribution / rigidBody.mass;
			
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
