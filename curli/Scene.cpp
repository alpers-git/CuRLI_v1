#include <Scene.h>
#include <GLFWHandler.h>
#include <lodepng.h>


void scheduleSynchForBuffers(entt::registry& registry, entt::entity e)
{

	//check if entity has a trimesh
	if (registry.any_of<CTriMesh, CBoundingBox>(e) && registry.all_of<CVertexArrayObject>(e))
	{
		Event event;
		event.type = Event::Type::GeometryChange;
		GLFWHandler::GetInstance().QueueEvent(event);
	}
	
}

Scene::Scene()
{
	//create sinks for trimesh and VAO components
	registry.on_construct<CVertexArrayObject>().connect<&scheduleSynchForBuffers>();
	registry.on_update<CVertexArrayObject>().connect<&scheduleSynchForBuffers>();
	registry.on_construct<CTriMesh>().connect<&scheduleSynchForBuffers>();
}

Scene::~Scene()
{
}

void CTransform::Update()
{
}

void CTriMesh::Update()
{
}

void CLight::Update()
{
}

void CVelocityField2D::Update()
{
}


void CForceField2D::Update()
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
	velocity = velocity + 0.5f * stepSize * acceleration
		+ glm::pow(glm::length(velocity + 0.0000001f), 2.0f)
		* drag * -glm::normalize(velocity + 0.0000001f) *  stepSize;
	
	position = position + 0.5f * stepSize * midVelocity;
}

void CBoundingBox::Update(){}

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
{
}

void CTexture2D::Update()
{
}

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
					const float stepSize = 0.33f;
					for (float step = 0; step < 1.0f; step+=stepSize)
					{
						if (!explicit_euler)
						{
							const float r = glm::length(rigidBody.position);
							const float deltaAngle = glm::length(rigidBody.velocity) *
								stepSize/r;
							glm::vec3 pPrime = glm::eulerAngleY(deltaAngle) * 
								glm::vec4(rigidBody.position,1.0f);
							forceFContribution = fField.ForceAt(
								glm::vec2(pPrime.x, pPrime.z)) * stepSize;
							//rigidBody.velocity = glm::vec3(0.0f);
							//rigidBody.acceleration = forceFContribution / rigidBody.mass;
							rigidBody.velocity += forceFContribution / rigidBody.mass * stepSize 
								+ glm::pow(glm::length(rigidBody.velocity + 0.0000001f), 2.0f)
								* rigidBody.drag * -glm::normalize(rigidBody.velocity + 0.0000001f) * 0.5f * stepSize;
							rigidBody.position += stepSize * rigidBody.velocity;
							//rigidBody.Update();
						}
						else
						{
							//midpoint method
							forceFContribution = fField.ForceAt(glm::vec2(rigidBody.position.x, rigidBody.position.z));
							const glm::vec3 acceleration = forceFContribution / rigidBody.mass;
							const glm::vec3 midVelocity = rigidBody.velocity + 0.5f * stepSize * acceleration
								+ glm::pow(glm::length(rigidBody.velocity + 0.0000001f), 2.0f)
								* rigidBody.drag * -glm::normalize(rigidBody.velocity + 0.0000001f) * 0.5f * stepSize;
							const glm::vec3 midPosition = rigidBody.position + 0.5f * stepSize * midVelocity;
							rigidBody.velocity = rigidBody.velocity + 0.5f * stepSize * acceleration
								+ glm::pow(glm::length(rigidBody.velocity + 0.0000001f), 2.0f)
								* rigidBody.drag * -glm::normalize(rigidBody.velocity + 0.0000001f) * stepSize;

							rigidBody.position = rigidBody.position + 0.5f * stepSize * midVelocity;
						}
					}
					i++;
				});
			/*if (i != 0)
				rigidBody.acceleration = forceFContribution / rigidBody.mass;*/
			
			registry.view<CBoundingBox>().each([&](CBoundingBox bbox) {
				bbox.Rebound(rigidBody);
			});
			if (i == 0)
				rigidBody.Update();
			transform.SetPosition(rigidBody.position);
			//transform.SetEulerRotation(rigidBody.rotation);
			transform.Update();
		});
	//registry.view<CTransform>().each([](CTransform& transform) { transform.Update(); });
	registry.view<CTriMesh>().each([&](CTriMesh& mesh) { mesh.Update(); });
	registry.view<CLight>().each([&](CLight& light) { light.Update(); });

	registry.view<CBoundingBox>().each([&](const entt::entity& entity, CBoundingBox& bb) 
		{
			if (bb.IsDirty() && EntityHas(entity, CType::VAO))
			{
				registry.replace<CVertexArrayObject>(entity);
			}
			bb.dirty = false;
		});
	
}

bool Scene::EntityHas(entt::entity e, CType component)
{
	switch (component)
	{
	case CType::Transform:
		return registry.all_of<CTransform>(e);
		break;
	case CType::TriMesh:
		return registry.all_of<CTriMesh>(e);
		break;
	case CType::PhongMaterial:
		return registry.all_of<CPhongMaterial>(e);
		break;
	case CType::Light:
		return registry.all_of<CLight>(e);
		break;
	case CType::VAO:
		return registry.all_of<CVertexArrayObject>(e);
		break;
	case CType::BoundingBox:
		return registry.all_of<CBoundingBox>(e);
		break;
	case CType::VelocityField2D:
		return registry.all_of<CVelocityField2D>(e);
		break;
	case CType::ForceField2D:
		return registry.all_of<CForceField2D>(e);
		break;
	case CType::RigidBody:
		return registry.all_of<CRigidBody>(e);
		break;
	case CType::Count:
		break;
	default:
		return false;
		break;
	}
	return false;
}

entt::entity Scene::CreateBoundingBox(glm::vec3 min, glm::vec3 max, GLuint programID)
{
	auto entity = CreateSceneObject("boundingbox");
	registry.emplace<CBoundingBox>(entity, min, max);
	registry.emplace<CVertexArrayObject>(entity);
	if (programID != -1)
	{
		CVertexArrayObject& vao = GetComponent<CVertexArrayObject>(entity);
		vao.CreateVAO();
		glm::vec3 vertexData[24]
		{
			min,
			{max.x, min.y, min.z},
		
			{max.x, min.y, min.z},
			{max.x, max.y, min.z},
		
			{max.x, max.y, min.z},
			{min.x, max.y, min.z},
		
			{min.x, max.y, min.z},
			min,

			{min.x, max.y, min.z},
			{min.x, max.y, max.z},
		
			{min.x, max.y, max.z},
			{min.x, min.y, max.z},
		
			{min.x, min.y, max.z},
			min,
		
			{min.x, min.y, max.z},
			{max.x, min.y, max.z},

			{max.x, min.y, max.z},
			{max.x, min.y, min.z},

			{max.x, min.y, max.z},
			max,

			max,
			{max.x, max.y, min.z},

			max,
			{min.x, max.y, max.z}
		};
	
		VertexBufferObject vertexVBO(
			(void*)vertexData,
			24,
			GL_FLOAT,
			"pos",
			3,
			programID);
		vao.AddVBO(vertexVBO);
	}

	return entity;
	
}

entt::entity Scene::CreatePointLight(glm::vec3 pos, float intesity, glm::vec3 color)
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
	registry.emplace<CVertexArrayObject>(entity);
	
	//InsertSceneObject(name, entity);

	return entity;
}

entt::entity Scene::CreateModelObject(cy::TriMesh& mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateSceneObject("unnamed-");
	registry.emplace<CTriMesh>(entity, mesh);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	registry.emplace<CPhongMaterial>(entity);
	registry.emplace<CVertexArrayObject>(entity);
	//InsertSceneObject("unnamed-", entity);
	return entity;
}

void CRigidBody::ApplyForce(glm::vec3 force)
{
	velocity += force / mass;
}


void CBoundingBox::Rebound(CRigidBody& rigidBody)
{
	glm::vec3 reboundNormal = glm::normalize(glm::vec3(
		(rigidBody.position.x < min.x ? 1.0f : (rigidBody.position.x > max.x ? -1.0f : 0.0f)),
		(rigidBody.position.y < min.y ? 1.0f : (rigidBody.position.y > max.y ? -1.0f : 0.0f)),
		(rigidBody.position.z < min.z ? 1.0f : (rigidBody.position.z > max.z ? -1.0f : 0.0f))
	));

	if (!glm::any(glm::isnan(reboundNormal)) && reboundNormal != glm::vec3(0.0f))
	{
		rigidBody.position += reboundNormal * 0.01f;
		rigidBody.velocity = glm::reflect(rigidBody.velocity, reboundNormal);
		rigidBody.acceleration = glm::reflect(rigidBody.acceleration, reboundNormal);
	}
}
