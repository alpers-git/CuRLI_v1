#include <Scene.h>

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

void CTransform::Update()
{
	
}

void CTriMesh::Update()
{
}
