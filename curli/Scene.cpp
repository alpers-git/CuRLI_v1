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
	return entity;
}

entt::entity Scene::CreateModelObject(cy::TriMesh& mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateEntity();
	registry.emplace<CTriMesh>(entity, mesh);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	return entity;
}

