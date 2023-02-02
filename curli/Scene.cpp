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

