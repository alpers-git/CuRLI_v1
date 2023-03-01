#include <Scene.h>
#include <GLFWHandler.h>


//===============Sinks for entity management===============
void scheduleSynchForBuffers(entt::registry& registry, entt::entity e)
{

	////check if entity has a trimesh
	if (registry.any_of<CTriMesh>(e))
	{
		Event event;
		event.type = Event::Type::GeometryChange;
		event.geometryChange.e = e;
		GLFWHandler::GetInstance().QueueEvent(event);
	}
	
}

void scheduleSychForAddedTexture(entt::registry& registry, entt::entity e)
{
	Event event;
	event.type = Event::Type::TextureChange;
	event.textureChange.e = e;
	event.textureChange.toBeRemoved = false;
	GLFWHandler::GetInstance().QueueEvent(event);
}

void scheduleSychForRemovedTexture(entt::registry& registry, entt::entity e)
{
	Event event;
	event.type = Event::Type::TextureChange;
	event.textureChange.e = e;
	event.textureChange.toBeRemoved = true;
	GLFWHandler::GetInstance().QueueEvent(event);
}

void synchTransformAndRigidBody(entt::registry& registry, entt::entity e)
{
	//check if entity has a rigid body
	if (registry.any_of<CRigidBody>(e))
	{
		//check if entity has a transform
		if (registry.any_of<CTransform>(e))
		{
			CTransform& transform = registry.get<CTransform>(e);
			CRigidBody& rigidBody = registry.get<CRigidBody>(e);
			rigidBody.position = transform.GetPosition();
			rigidBody.rotation = transform.GetRotation();
		}
	}
}

void synchEnvMap(entt::registry& registry, entt::entity e)
{
	//------ensure single environment map in scene------//
	//check if scene has another environment map
	const auto view = registry.view<CSkyBox>();
	if (view.size()>1)
	{
		//then remove it
		registry.destroy(view.front());
	}
	
	//schedule update with renderer
	Event event;
	event.type = Event::Type::GeometryChange;
	event.geometryChange.e = e;
	GLFWHandler::GetInstance().QueueEvent(event);
	
	Event event2;
	event2.type = Event::Type::TextureChange;
	event2.textureChange.e = e;
	event2.textureChange.toBeRemoved = false;
	GLFWHandler::GetInstance().QueueEvent(event2);
}

//==============================
Scene::Scene()
{
	registry.on_construct<CTriMesh>().connect<&scheduleSynchForBuffers>();
	registry.on_update<CTriMesh>().connect<&scheduleSynchForBuffers>();

	registry.on_destroy<CImageMaps>().connect<&scheduleSychForRemovedTexture>();
	
	registry.on_construct<CRigidBody>().connect<&synchTransformAndRigidBody>();
	registry.on_update<CRigidBody>().connect<&synchTransformAndRigidBody>();
	registry.on_construct<CTransform>().connect<&synchTransformAndRigidBody>();//Untested!


	registry.on_construct<CSkyBox>().connect<&synchEnvMap>();
	registry.on_update<CSkyBox>().connect<&synchEnvMap>();
	
}

Scene::~Scene()
{
}

void CTransform::Update()
{
}

void CTriMesh::InitializeFrom(cy::TriMesh& mesh)
{
	shading = ShadingMode::PHONG;
	unsigned int minAttribCount = glm::min(mesh.NV(), glm::min(mesh.NVN(), mesh.NVT()));
	this->vertices.resize(minAttribCount);
	std::fill(this->vertices.begin(), this->vertices.end(), glm::vec3(NAN));
	this->vertexNormals.resize(minAttribCount);
	std::fill(this->vertexNormals.begin(), this->vertexNormals.end(), glm::vec3(NAN));
	this->textureCoords.resize(minAttribCount);
	std::fill(this->textureCoords.begin(), this->textureCoords.end(), glm::vec2(NAN));
	this->faces.resize(mesh.NF());
	for (size_t i = 0; i < mesh.NF(); i++)
	{
		bool pushNewFace = this->vertices.size() < mesh.F(i).v[0] || 
			this->vertices.size() < mesh.F(i).v[1] || 
			this->vertices.size() < mesh.F(i).v[2] ||
			this->vertexNormals.size() < mesh.FN(i).v[0] ||
			this->vertexNormals.size() < mesh.FN(i).v[1] ||
			this->vertexNormals.size() < mesh.FN(i).v[2] ||
			this->textureCoords.size() < mesh.FT(i).v[0] ||
			this->textureCoords.size() < mesh.FT(i).v[1] ||
			this->textureCoords.size() < mesh.FT(i).v[2];
		if (!pushNewFace)
		{
			const bool repeatedVerts = !glm::all(glm::isnan(this->vertices[mesh.F(i).v[0]])) &&
				!glm::all(glm::isnan(this->vertices[mesh.F(i).v[1]])) &&
				!glm::all(glm::isnan(this->vertices[mesh.F(i).v[2]]));
			const bool repeatedNormals = !glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[0]])) &&
				!glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[1]])) &&
				!glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[2]]));
			const bool repeatedTxCoords = !glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[0]])) &&
				!glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[1]])) &&
				!glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[2]]));
			pushNewFace = repeatedVerts || repeatedNormals || repeatedTxCoords;
		}
			
		glm::ivec3 face = glm::ivec3(mesh.F(i).v[0], mesh.F(i).v[1], mesh.F(i).v[2]);
		glm::ivec3 tface = glm::ivec3(mesh.FT(i).v[0], mesh.FT(i).v[1], mesh.FT(i).v[2]);
		glm::ivec3 nface = glm::ivec3(mesh.FN(i).v[0], mesh.FN(i).v[1], mesh.FN(i).v[2]);
		if (!pushNewFace)//all of the vertex attribues of these faces are NaN... then we push them as is
		{
			this->faces[i] = face;
		
			for (size_t j = 0; j < 3; j++)
			{
				glm::vec3 vertex = glm::cy2GLM(mesh.V(face[j]));
				this->vertices[face[j]] = vertex;
				assert(mesh.NVN() > nface[j]);
				glm::vec3 normal = glm::cy2GLM(mesh.VN(nface[j]));
				this->vertexNormals[face[j]] = normal;
				assert(mesh.NVT() > tface[j]);
				{
					glm::vec2 texCoord = glm::cy2GLM(mesh.VT(tface[j]));
					this->textureCoords[face[j]] = texCoord;
				}
			}
		}
		else //since at least one vertex attribute was not NaN 
		{	//we need to create a new face and duplicate non unique attributes
			this->faces.push_back(glm::uvec3(this->vertices.size(), 
				this->vertices.size()+1, 
				this->vertices.size()+2));
			for (size_t j = 0; j < 3; j++)
			{
				glm::vec3 vertex = glm::cy2GLM(mesh.V(mesh.F(i).v[j]));
				this->vertices.push_back(vertex);
				assert(mesh.NVN() > nface[j]);
				{
					glm::vec3 normal = glm::cy2GLM(mesh.VN(nface[j]));
					this->vertexNormals.push_back(normal);
				}
				assert (mesh.NVT() > tface[j]);
				{
					glm::vec2 texCoord = glm::cy2GLM(mesh.VT(tface[j]));
					this->textureCoords.push_back(texCoord);
				}
			}
		}
	}
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

void CRigidBody::Update()
{
}

void CForceField2D::Update()
{
}
void CBoundingBox::Update()
{
}

void CSkyBox::Update()
{
}

glm::vec3 CVelocityField2D::At(glm::vec3 p)
{
	switch (this->plane)
	{
	case FieldPlane::XY:
		return scaling * glm::vec3(field({ p.x, p.y }), 0.0f);
		break;
	case FieldPlane::XZ:
	{
		auto t = field(glm::vec2(p.x, p.z));
		return scaling * glm::vec3(t.x, 0.0f, t.y);
		break;
	}
	case FieldPlane::YZ:
	{
		auto t = field(glm::vec2(p.y, p.z));
		return scaling * glm::vec3(0.0f, t.x, t.y);
		break;
	}
	default:
		return glm::vec3(0.0f);
		break;
	}
}

glm::vec3 CForceField2D::At(glm::vec3 p)
{
	switch (this->plane)
	{
	case FieldPlane::XY:
		return scaling * glm::vec3(field({p.x, p.y}), 0.0f);
		break;
	case FieldPlane::XZ:
	{
		auto t = field(glm::vec2(p.x, p.z));
		return scaling * glm::vec3(t.x, 0.0f, t.y);
		break;
	}
	case FieldPlane::YZ:
	{
		auto t = field(glm::vec2(p.y, p.z));
		return scaling * glm::vec3(0.0f, t.x, t.y);
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

void CImageMaps::Update()
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
			
			
			/*registry.view<CBoundingBox>().each([&](CBoundingBox bbox) {
				bbox.Rebound(rigidBody);
			});*/

			transform.SetPosition(rigidBody.position);
			//transform.SetEulerRotation(rigidBody.rotation);
			transform.Update();
		});
	registry.view<CTriMesh>().each([&](CTriMesh& mesh) { mesh.Update(); });
	registry.view<CLight>().each([&](CLight& light) { light.Update(); });

	registry.view<CBoundingBox>().each([&](const entt::entity& entity, CBoundingBox& bb) 
		{
			bb.dirty = false;
		});

	registry.view<CImageMaps>().each([&](const entt::entity& entity, CImageMaps& maps)
		{
			if (maps.dirty)
			{
				Event e;
				e.type = Event::Type::TextureChange;
				e.textureChange.e = entity;
				e.textureChange.toBeRemoved = false;
			
				GLFWHandler::GetInstance().QueueEvent(e);
				maps.dirty = false;
			}
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
	case CType::ImageMaps:
		return registry.all_of<CImageMaps>(e);
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
	if (programID != -1)
	{
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
	
	cy::TriMesh tmpMesh;
	tmpMesh.LoadFromFileObj(meshPath.c_str());
	auto mesh = registry.emplace<CTriMesh>(entity, tmpMesh);
	auto& transform = registry.emplace<CTransform>(entity, position, rotation, scale);
	transform.SetPivot(mesh.GetBoundingBoxCenter());
	auto& material = registry.emplace<CPhongMaterial>(entity);
	
	
	if (tmpMesh.NM() > 0)
	{
		material.ambient = glm::make_vec3(tmpMesh.M(0).Ka);
		material.diffuse = glm::make_vec3(tmpMesh.M(0).Kd);
		material.specular = glm::make_vec3(tmpMesh.M(0).Ks);
		
		std::string path = meshPath.substr(0, meshPath.find_last_of("/\\") + 1);
		
		/*if (!mesh.GetMatAmbientTexture(0).empty())
			registry.emplace<CTexture2D>(entity, path + mesh.GetMatAmbientTexture(0));*/
		if (tmpMesh.M(0).map_Kd != NULL || tmpMesh.M(0).map_Ks != NULL)
		{
			auto& textures = registry.emplace<CImageMaps>(entity);

			if (tmpMesh.M(0).map_Kd != NULL)
				textures.AddImageMap(ImageMap::BindingSlot::T_DIFFUSE, path + std::string(tmpMesh.M(0).map_Kd));
			
			if (tmpMesh.M(0).map_Ks != NULL)
				textures.AddImageMap(ImageMap::BindingSlot::T_SPECULAR, path + std::string(tmpMesh.M(0).map_Ks));
			
		}
			
	}
	

	return entity;
}

entt::entity Scene::CreateModelObject(cy::TriMesh& mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	auto entity = CreateSceneObject("unnamed-");
	registry.emplace<CTriMesh>(entity, mesh);
	registry.emplace<CTransform>(entity, position, rotation, scale);
	registry.emplace<CPhongMaterial>(entity);
	return entity;
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

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, std::string path)
{
	imgMaps.insert({ slot, ImageMap(path, slot) });
	
	//TODO::Schedule texture synch with renderer
	dirty = true;
}

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, Camera camera, glm::uvec2 dims)
{
	imgMaps.insert({ slot, ImageMap(camera, dims, slot) });

	//TODO::Schedule texture synch with renderer
	dirty = true;
}

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, std::string path[6])
{
	imgMaps.insert({ slot, ImageMap(path, slot) });
	dirty = true;
}

void CImageMaps::RemoveImageMap(ImageMap::BindingSlot slot)
{
	imgMaps.erase(slot);
	
	//TODO::Schedule texture synch with renderer
	dirty = true;
}

std::string ImageMap::GetSlotName()
{
	switch (bindingSlot)
	{
	case ImageMap::BindingSlot::T_AMBIENT:
		return "Ambient texture";
		break;
	case ImageMap::BindingSlot::T_DIFFUSE:
		return "Diffuse texture";
		break;
	case ImageMap::BindingSlot::T_SPECULAR:
		return "Specular texture";
		break;
	case ImageMap::BindingSlot::NORMAL:
		return "NormalMap";
		break;
	case ImageMap::BindingSlot::BUMP:
		return "BumpMap";
		break;
	case ImageMap::BindingSlot::ENV_MAP:
		return "Environment Map";
		break;
	default:
		return "";
		break;
	}
}

std::vector<unsigned char> CSkyBox::GetSideImagesFlat()
{
	std::vector<unsigned char> flattenedImages;
	flattenedImages.reserve(sides[0].GetImage().size() * 6);
	for (auto& img : sides)
	{
		flattenedImages.insert(flattenedImages.end(), img.GetImage().begin(), img.GetImage().end());
	}
	return flattenedImages;
}
