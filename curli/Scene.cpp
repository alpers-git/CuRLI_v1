#include <Scene.h>
#include <GLFWHandler.h>
#include <Eigen/Eigenvalues>

//===============Sinks for entity management===============
void scheduleSynchForAddedGeom(entt::registry& registry, entt::entity e)
{

	////check if entity has a trimesh
	if (registry.any_of<CTriMesh>(e))
	{
		Event event;
		event.type = Event::Type::GeometryChange;
		event.geometryChange.e = e;
		event.geometryChange.toBeRemoved = false;
		GLFWHandler::GetInstance().QueueEvent(event);
	}
	
}

void scheduleSynchForRemovedGeom(entt::registry& registry, entt::entity e)
{

	////check if entity has a trimesh
	if (registry.any_of<CTriMesh>(e))
	{
		Event event;
		event.type = Event::Type::GeometryChange;
		event.geometryChange.e = e;
		event.geometryChange.toBeRemoved = true;
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
			rigidBody.SetRotation(transform.GetRotation());
			transform.useRotationMatrix = true;
			CTriMesh& mesh = registry.get<CTriMesh>(e);
			rigidBody.initializeInertiaTensor(&mesh, &transform);
			rigidBody.SetMassMatrix();
		}
	}
}

void rigidBodyRemoved(entt::registry& registry, entt::entity e)
{
	if (registry.any_of<CRigidBody>(e))
	{
		//check if entity has a transform
		if (registry.any_of<CTransform>(e))
		{
			CTransform& transform = registry.get<CTransform>(e);
			transform.useRotationMatrix = false;
		}
	}
}

void synchSkyBox(entt::registry& registry, entt::entity e)
{
	//------ensure single skybox in scene------//
	//check if scene has another skybox
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
	event.geometryChange.toBeRemoved = false;
	GLFWHandler::GetInstance().QueueEvent(event);
	
	Event event2;
	event2.type = Event::Type::TextureChange;
	event2.textureChange.e = e;
	event2.textureChange.toBeRemoved = false;
	GLFWHandler::GetInstance().QueueEvent(event2);
}

void synchPhysicsBounds(entt::registry& registry, entt::entity e)
{
	//------ensure single skybox in scene------//
	//check if scene has another skybox
	const auto view = registry.view<CPhysicsBounds>();
	if (view.size() > 1)
	{
		//then remove it
		registry.destroy(view.front());
	}
}

//==============================
Scene::Scene()
{
	registry.on_construct<CTriMesh>().connect<&scheduleSynchForAddedGeom>();
	registry.on_update<CTriMesh>().connect<&scheduleSynchForAddedGeom>();
	registry.on_destroy<CTriMesh>().connect<&scheduleSynchForRemovedGeom>();

	registry.on_destroy<CImageMaps>().connect<&scheduleSychForRemovedTexture>();
	
	registry.on_construct<CRigidBody>().connect<&synchTransformAndRigidBody>();
	registry.on_update<CRigidBody>().connect<&synchTransformAndRigidBody>();
	registry.on_construct<CTransform>().connect<&synchTransformAndRigidBody>();//Untested!
	registry.on_update<CTransform>().connect<&synchTransformAndRigidBody>();//Untested!
	
	registry.on_destroy<CRigidBody>().connect<&rigidBodyRemoved>();


	registry.on_construct<CSkyBox>().connect<&synchSkyBox>();
	registry.on_update<CSkyBox>().connect<&synchSkyBox>();

	registry.on_construct<CPhysicsBounds>().connect<&synchPhysicsBounds>();
	registry.on_update<CPhysicsBounds>().connect<&synchPhysicsBounds>();
	
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
	bool hasTextureCoords = true;
	if (minAttribCount == 0)
	{
		if(mesh.NV())
			throw std::runtime_error("Mesh has no vertices");
		if (mesh.NVN())
		{
			mesh.ComputeNormals();
			minAttribCount = glm::min(mesh.NV(), glm::min(mesh.NVN(), mesh.NVT()));
		}
		if (mesh.NVT())
		{
			hasTextureCoords = false;
			minAttribCount = glm::min(mesh.NV(), mesh.NVN());
		}
	}
	this->vertices.resize(minAttribCount);
	std::fill(this->vertices.begin(), this->vertices.end(), glm::vec3(NAN));
	this->vertexNormals.resize(minAttribCount);
	std::fill(this->vertexNormals.begin(), this->vertexNormals.end(), glm::vec3(NAN));
	this->textureCoords.resize(minAttribCount);
	if(hasTextureCoords) std::fill(this->textureCoords.begin(), this->textureCoords.end(), glm::vec2(NAN));
	this->faces.resize(mesh.NF());
	for (size_t i = 0; i < mesh.NF(); i++)
	{
		bool pushNewFace = this->vertices.size() < mesh.F(i).v[0] ||
			this->vertices.size() <= mesh.F(i).v[1] ||
			this->vertices.size() <= mesh.F(i).v[2] ||
			this->vertexNormals.size() <= mesh.FN(i).v[0] ||
			this->vertexNormals.size() <= mesh.FN(i).v[1] ||
			this->vertexNormals.size() <= mesh.FN(i).v[2];
		if (hasTextureCoords)
			pushNewFace = pushNewFace ||
			this->textureCoords.size() <= mesh.FT(i).v[0] ||
			this->textureCoords.size() <= mesh.FT(i).v[1] ||
			this->textureCoords.size() <= mesh.FT(i).v[2];
		if (!pushNewFace)
		{
			const bool repeatedVerts = !glm::all(glm::isnan(this->vertices[mesh.F(i).v[0]])) &&
				!glm::all(glm::isnan(this->vertices[mesh.F(i).v[1]])) &&
				!glm::all(glm::isnan(this->vertices[mesh.F(i).v[2]]));
			const bool repeatedNormals = !glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[0]])) &&
				!glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[1]])) &&
				!glm::all(glm::isnan(this->vertexNormals[mesh.FN(i).v[2]]));
			const bool repeatedTxCoords = hasTextureCoords &&
				!glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[0]])) &&
				!glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[1]])) &&
				!glm::all(glm::isnan(this->textureCoords[mesh.FT(i).v[2]]));
			pushNewFace = repeatedVerts || repeatedNormals || repeatedTxCoords;
		}
			
		glm::ivec3 face = glm::ivec3(mesh.F(i).v[0], mesh.F(i).v[1], mesh.F(i).v[2]);
		glm::ivec3 nface = glm::ivec3(mesh.FN(i).v[0], mesh.FN(i).v[1], mesh.FN(i).v[2]);
		glm::ivec3 tface = hasTextureCoords ? glm::ivec3(mesh.FT(i).v[0], mesh.FT(i).v[1], mesh.FT(i).v[2]) : glm::ivec3();
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

	//go over vertices and if they are Nan remove them and update faces accordingly
	for (int i = 0; i < this->vertices.size(); i++)
	{
		if (glm::any(glm::isnan(this->vertices[i])))
		{
			this->vertices.erase(this->vertices.begin() + i);
			this->vertexNormals.erase(this->vertexNormals.begin() + i);
			if(hasTextureCoords)
				this->textureCoords.erase(this->textureCoords.begin() + i);
			for (int j = 0; j < this->faces.size(); j++)
			{
				if (this->faces[j].x > i)
					this->faces[j].x--;
				if (this->faces[j].y > i)
					this->faces[j].y--;
				if (this->faces[j].z > i)
					this->faces[j].z--;
			}
			i--;
		}
	}
}

void parseFileLine(std::ifstream& file, std::string& line)
{
	while (true)
	{
		std::getline(file, line);
		if (line[0] != '#')//skip comments
			break;
	}
}

void CTriMesh::GenerateFaceFrom(const glm::vec3 v0,const glm::vec3 v1,const glm::vec3 v2,
	const glm::vec3 vIn, const glm::ivec3 vertIndices)
{
	glm::ivec3 face;
	//compute the normal for the face
	const glm::vec3 e1 = v1 - v0;
	const glm::vec3 e2 = v2 - v0;
	glm::vec3 normal = glm::normalize(glm::cross(e2, e1));
	const glm::vec3 inwardVec = vIn - (v0 +
		v1 + v2) / 3.0f;
	if (glm::dot(normal, -inwardVec) >= 0)
		normal = -normal;
	glm::vec3 boundaryVertices[3] = {v0,v1,v2};
	for (int j = 0; j < 3; j++)
	{
		//check if index of the boundary vertex is already in the mesh faces using tetgen2Face map
		//auto foundIndex = tetgen2Face.find(indices[j]);
		if (vertIndices[j] == -1)//new vertex
		{
			//Position
			this->vertices.push_back(boundaryVertices[j]);

			//Normals
			this->vertexNormals.push_back(normal);

			//Texture
			this->textureCoords.push_back({ 0,0 });//insert empty coords

			face[j] = this->vertices.size() - 1;
			//tetgen2Face.insert(std::make_pair(indices[j], face[j]));
		}
		else//existing vertex
		{
			face[j] = vertIndices[j];
		}
	}
	this->faces.push_back(face);
}

struct TripleHash {
	size_t operator()(const glm::ivec3& t) const {
		int x = (t.x);
		int y = (t.y);
		int z = (t.z);
		int a[3] = { x, y, z };
		std::sort(a, a + 3);
		size_t h1 = std::hash<int>{}(a[0]);
		size_t h2 = std::hash<int>{}(a[1]);
		size_t h3 = std::hash<int>{}(a[2]);
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

// Function to mark boundary nodes
void mark_boundary_nodes(
	const std::vector<glm::ivec4>& elements,
	std::vector<bool>& boundary)
{
	// Create a hash map to count the number of times each face is encountered
	std::unordered_map<glm::ivec3, int, TripleHash> face_count;

	// Count the number of times each face is encountered
	for (const auto& tetrahedron : elements) {
		// Get the indices of the tetrahedron's vertices
		int i = tetrahedron.x;
		int j = tetrahedron.y;
		int k = tetrahedron.z;
		int l = tetrahedron.w;

		// For each face of the tetrahedron, increment its count in the hash map
		++face_count[glm::ivec3(i, k, j)];
		++face_count[glm::ivec3(i, l, j)];
		++face_count[glm::ivec3(i, l, k)];
		++face_count[glm::ivec3(j, l, k)];
	}

	// Find the boundary nodes
	for (const auto& [face, count] : face_count) {
		// If the face is encountered only once, mark its vertices as boundary nodes
		if (count == 1) {
			int i = face.x;
			int j = face.y;
			int k = face.z;
			boundary[i] = true;
			boundary[j] = true;
			boundary[k] = true;
		}
	}
}

void CTriMesh::InitializeFrom(const std::string& nodePath, const std::string elePath)
{
	printf("======Reading Tetgen files=====\n");
	//check the file extensions node needs to be .node and ele needs to be .ele
	if (nodePath.substr(nodePath.find_last_of(".") + 1) != "node" ||
		elePath.substr(elePath.find_last_of(".") + 1) != "ele")
	{
		throw std::exception("Invalid extension for tetgen files");
	}
	
	//------------------node file----------------------
	//open the node file
	std::ifstream nodeFile(nodePath, std::ios::in);
	if (!nodeFile.is_open())
	{
		throw std::exception("Could not open node file");
	}
	//first line includes space seperated # nodes, # dimensions, # attributes, # boundary markers
	std::string line;
	parseFileLine(nodeFile, line);
	std::istringstream iss(line);
	std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{} };
	const unsigned int numNodes = std::stoi(tokens[0]);
	const unsigned int numDimensions = std::stoi(tokens[1]);
	const unsigned int numAttributes = std::stoi(tokens[2]);
	const unsigned int numBoundaryMarkers = std::stoi(tokens[3]);

	if (numDimensions != 3)
		throw std::exception("Only 3D meshes are supported");
	if(numNodes <=0)
		throw std::exception("Invalid number of nodes");
	
	std::vector<glm::vec3> nodes;
	std::vector<bool> boundary;
	nodes.resize(numNodes);
	boundary.resize(numNodes, false);

	//go over the file and find the min index
	int minIndex = 2;
	for (int i = 0; i < numNodes; i++)
	{
		parseFileLine(nodeFile, line);
		std::istringstream iss(line);
		std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>{} };
		if (minIndex > std::stoi(tokens[0]))
			minIndex = std::stoi(tokens[0]);
	}

	//rewind to begining of the file and skip the first line
	nodeFile.seekg(0);
	nodeFile.clear();
	parseFileLine(nodeFile, line);

	//read the rest of the file with a loop and store the vertices.
	for (int i = 0; i < numNodes; i++)
	{
		parseFileLine(nodeFile, line);
		std::istringstream iss(line);
		std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>{} };
		glm::vec3 node = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
		nodes.at(std::stof(tokens[0])- minIndex) = node;
		if(numBoundaryMarkers > 0)
			boundary.at(std::stof(tokens[0]) - minIndex) = std::stoi(tokens[4]) == 1;// 1 means boundary vert
	}
	nodeFile.close();

	//------------------ele file----------------------
	std::ifstream eleFile(elePath, std::ios::in);
	if (!eleFile.is_open())
	{
		throw std::exception("Could not open ele file");
	}

	//first line includes space seperated # elements, # nodes per element, # attributes
	parseFileLine(eleFile, line);
	std::istringstream iss2(line);
	std::vector<std::string> tokens2{ std::istream_iterator<std::string>{iss2},
		std::istream_iterator<std::string>{} };
	const unsigned int numElements = std::stoi(tokens2[0]);
	const unsigned int numNodesPerElement = std::stoi(tokens2[1]);
	const unsigned int numAttributes2 = std::stoi(tokens2[2]);
	
	if (numNodesPerElement != 4)
		throw std::exception("Only tetrahedral meshes are supported");
	if (numElements <= 0)
		throw std::exception("Invalid number of elements");
	
	std::vector<glm::ivec4> elements;
	
	//read the rest of the file with a loop and store the vertices.
	
	for (int i = 0; i < numElements; i++)
	{
		parseFileLine(eleFile, line);
		std::istringstream iss(line);
		std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
			std::istream_iterator<std::string>{} };
		glm::ivec4 element = glm::ivec4(std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]));
		elements.push_back(element - glm::ivec4(minIndex));
	}
	eleFile.close();
	if (numBoundaryMarkers == 0)//well the file doesnt have a boundary marker...
	{
		mark_boundary_nodes(elements, boundary);
	}
	printf("======Done reading Tetgen files=====\n\t# nodes:%d \n\t# elements:%d\n", numNodes, numElements);
	printf("======Generating surface mesh=====\n");
	//generate the mesh
	//A map to input data to our face structure
	std::unordered_map<int, int> tetgen2Face;
	//go over tets if they have a boundary nodes push them as a face
	for (int i = 0; i < elements.size(); i++)
	{
		const glm::ivec4 indices = elements.at(i);

		glm::vec3 boundaryVertices[4] = {glm::vec3(NAN),glm::vec3(NAN),glm::vec3(NAN), glm::vec3(NAN)};
		int boundaryCount = 0;//for indexing the array up there
		glm::vec3 inwardVertex;
		for (int j = 0; j < 4; j++)
		{
			if (boundary.at(indices[j]))
				boundaryVertices[boundaryCount++] = nodes.at(indices[j]);
			else
				inwardVertex = nodes.at(indices[j]);
		}
		//if any of the boundary vertices is nan this tet is not a boundary tet, skip!
		if (boundaryCount<3)
			continue;
		if (boundaryCount == 3)
		{
			glm::ivec3 indicesForVertices(-1, -1, -1);//init as new vertices, so no known index(-1)
			int numNewVert = 0;
			//for (int j = 0; j < 3; j++)//check if that vertex already exists
			//{
			//	auto foundIndex = tetgen2Face.find(indices[j]);
			//	if(foundIndex != tetgen2Face.end())//duplicate vertex so get the values from the map
			//		indicesForVertices[j] = foundIndex->second;
			//	else
			//	{
			//		tetgen2Face.insert(std::make_pair(indices[j], this->vertices.size() + numNewVert));
			//		numNewVert++;
			//	}
			//}
			GenerateFaceFrom(boundaryVertices[0], boundaryVertices[1], boundaryVertices[2],
				inwardVertex, indicesForVertices);

		}
		if (boundaryCount == 4)
		{
			//push all 4 faces...
			//-------------0-------------
			glm::ivec3 curIndices = { indices[0], indices[1], indices[2] };
			inwardVertex = boundaryVertices[3];
			glm::ivec3 indicesForVertices(-1, -1, -1);//init as new vertices, so no known index(-1)
			int numNewVert = 0;
			//for (int j = 0; j < 3; j++)//check if that vertex already exists
			//{
			//	auto foundIndex = tetgen2Face.find(curIndices[j]);
			//	if (foundIndex != tetgen2Face.end())
			//		indicesForVertices[j] = foundIndex->second;
			//	else
			//	{
			//		tetgen2Face.insert(std::make_pair(curIndices[j], this->vertices.size() + numNewVert));
			//		numNewVert++;
			//	}
			//}
			GenerateFaceFrom(boundaryVertices[0], boundaryVertices[1], boundaryVertices[2],
				inwardVertex, indicesForVertices);
			//-------------1-------------
			curIndices = { indices[1], indices[3], indices[2] };
			inwardVertex = boundaryVertices[0];
			indicesForVertices = glm::ivec3(-1, -1, -1);//init as new vertices, so no known index(-1)
			numNewVert = 0;
			//for (int j = 0; j < 3; j++)//check if that vertex already exists
			//{
			//	auto foundIndex = tetgen2Face.find(curIndices[j]);
			//	if (foundIndex != tetgen2Face.end())
			//		indicesForVertices[j] = foundIndex->second;
			//	else
			//	{
			//		tetgen2Face.insert(std::make_pair(curIndices[j], this->vertices.size() + numNewVert));
			//		numNewVert++;
			//	}
			//}
			GenerateFaceFrom(boundaryVertices[1], boundaryVertices[3], boundaryVertices[2],
				inwardVertex, indicesForVertices);
			//-------------2-------------
			curIndices = { indices[3], indices[0], indices[2] };
			inwardVertex = boundaryVertices[1];
			indicesForVertices = glm::ivec3(-1, -1, -1);//init as new vertices, so no known index(-1)
			numNewVert = 0;
			//for (int j = 0; j < 3; j++)//check if that vertex already exists
			//{
			//	auto foundIndex = tetgen2Face.find(curIndices[j]);
			//	if (foundIndex != tetgen2Face.end())
			//		indicesForVertices[j] = foundIndex->second;
			//	else
			//	{
			//		tetgen2Face.insert(std::make_pair(curIndices[j], this->vertices.size() + numNewVert));
			//		numNewVert++;
			//	}
			//}
			GenerateFaceFrom(boundaryVertices[3], boundaryVertices[0], boundaryVertices[2],
				inwardVertex, indicesForVertices);
			//-------------3-------------
			curIndices = { indices[0], indices[1], indices[3] };
			inwardVertex = boundaryVertices[2];
			indicesForVertices = glm::ivec3(-1, -1, -1);//init as new vertices, so no known index(-1)
			numNewVert = 0;
			//for (int j = 0; j < 3; j++)//check if that vertex already exists
			//{
			//	auto foundIndex = tetgen2Face.find(curIndices[j]);
			//	if (foundIndex != tetgen2Face.end())
			//		indicesForVertices[j] = foundIndex->second;
			//	else
			//	{
			//		tetgen2Face.insert(std::make_pair(curIndices[j], this->vertices.size() + numNewVert));
			//		numNewVert++;
			//		}
			//}
			GenerateFaceFrom(boundaryVertices[0], boundaryVertices[1], boundaryVertices[3],
				inwardVertex, indicesForVertices);
		}

	}

	printf("======Done=====\n\t#verts: %d\n\t#normals %d\n\t#text co: %d\n\t#faces: %d\n", 
		this->vertices.size(), this->vertexNormals.size(), this->textureCoords.size(), this->faces.size());
	
}

void CTriMesh::Update()
{
}

glm::vec3 CTriMesh::ApproximateTheClosestPointTo(glm::vec3 point, int stride)
{
	float minDist = std::numeric_limits<float>::max();
	glm::vec3 closestPoint = glm::vec3(0.0f);
	for (int i = 0; i < this->vertices.size(); i += stride)
	{
		glm::vec3 v = this->vertices[i];
		if (glm::any(glm::isnan(v)))
			continue;
		if(glm::distance(point, v) < minDist)
		{
			minDist = glm::distance(point, v);
			closestPoint = v;
		}
	}
	return closestPoint;
}

bool CLight::slotUsed[15];

void CLight::Update()
{
}

void CVelocityField2D::Update()
{
}

void CRigidBody::Update()
{
}

void CRigidBody::initializeInertiaTensor(const CTriMesh* mesh, CTransform* transform)
{
	//Assuming the object is uniform density
	Eigen::Matrix3f mat = Eigen::Matrix3f::Zero();
	inertiaAtRest = glm::mat3(0.f);
	
	//go over all the vertices, calculate the inertia tensor and sum
	for (size_t i = 0; i < mesh->GetNumVertices(); i++)
	{
		const glm::vec3 v = /*transform->GetModelMatrix() */
			glm::vec4(mesh->GetVertex(i), 1.0f);
		if (glm::all(glm::isnan(v)))
			continue;
		inertiaAtRest[0][0] = inertiaAtRest[0][0] + v.y * v.y + v.z * v.z;
		inertiaAtRest[1][1] = inertiaAtRest[1][1] + v.x * v.x + v.z * v.z;
		inertiaAtRest[2][2] = inertiaAtRest[2][2] + v.x * v.x + v.y * v.y;
		inertiaAtRest[0][1] = inertiaAtRest[0][1] - v.x * v.y;
		inertiaAtRest[0][2] = inertiaAtRest[0][2] - v.x * v.z;
		inertiaAtRest[1][2] = inertiaAtRest[1][2] - v.y * v.z;
	}
	inertiaAtRest = inertiaAtRest * mass;
	/*Eigen::EigenSolver<Eigen::Matrix3f> solver(mat, false);
	auto& eigenValues = solver.eigenvalues();

	inertiaAtRest = glm::mat3(0.0f);
	inertiaAtRest[0][0] = eigenValues(0).real();
	inertiaAtRest[1][1] = eigenValues(1).real();
	inertiaAtRest[2][2] = eigenValues(2).real();*/
}

void CRigidBody::TakeFwEulerStep(float dt)
{
	//linear
	const glm::vec3 _v = GetVelocity();
	const float mag = glm::length(_v);
	ApplyLinearImpulse(-0.5f * drag * mag * _v * dt);
	ApplyLinearImpulse(glm::vec3(0,-1,0) * gravity * mass * dt);
	const glm::vec3 v = GetVelocity();
	position += v * dt;

	//angular
	const glm::vec3 _w = GetAngularVelocity();
	const float magw = glm::length(_w);
	ApplyAngularImpulse(-0.5f * drag * magw * _w * dt * 100.f);
	const glm::vec3 w = GetAngularVelocity();
	orientationQuat += (0.5f * dt) * glm::quat(0.0f, w) * orientationQuat;
	orientationQuat = glm::normalize(orientationQuat);

	orientationMatrix = glm::toMat3(orientationQuat);
}

void CRigidBody::SetMassMatrix()
{
	invMassMatrix = glm::inverse(glm::mat3(1.0f) * mass);
}

void CRigidBody::ApplyLinearImpulse(glm::vec3 imp)
{
	linearMomentum += imp;
}

void CRigidBody::ApplyAngularImpulse(glm::vec3 imp)
{
	angularMomentum += imp;
}

void CForceField2D::Update()
{
}
void CPhysicsBounds::Update()
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
	registry.view<CImageMaps>().each([&](const entt::entity& entity, CImageMaps& maps)
		{
			if (maps.scheduledTextureUpdate)
			{
				Event e;
				e.type = Event::Type::TextureChange;
				e.textureChange.e = entity;
				e.textureChange.toBeRemoved = false;
			
				GLFWHandler::GetInstance().QueueEvent(e);
				maps.scheduledTextureUpdate = false;
			}
		});

	registry.view<CLight>().each([&](const entt::entity& entity, CLight& light)
		{
			if (light.scheduledTextureUpdate)
			{
				Event e;
				e.type = Event::Type::TextureChange;
				e.textureChange.e = entity;
				e.textureChange.toBeRemoved = !light.IsCastingShadows();

				GLFWHandler::GetInstance().QueueEvent(e);
				light.scheduledTextureUpdate = false;
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
	case CType::PhysicsBounds:
		return registry.all_of<CPhysicsBounds>(e);
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
	case CType::BoxCollider:
		return registry.all_of<CBoxCollider>(e);
		break;
	case CType::Count:
		break;
	default:
		return false;
		break;
	}
	return false;
}

std::vector<glm::vec3> CPhysicsBounds::GenerateVertices()
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
	std::vector<glm::vec3> vertices = std::vector<glm::vec3>(vertexData, vertexData + 24);
	return vertices;
}

entt::entity Scene::CreatePointLight(glm::vec3 pos, float intesity, glm::vec3 color)
{
	auto entity = CreateSceneObject("Point Light");
	registry.emplace<CLight>(entity, LightType::POINT, color, glm::min(intesity, 1.0f), pos, 
		glm::vec3(0, 0, 0), 0);
	return entity;
}

entt::entity Scene::CreateDirectionalLight(glm::vec3 dir, float intesity, glm::vec3 color)
{
	auto entity = CreateSceneObject("Directional Light");
	registry.emplace<CLight>(entity, LightType::DIRECTIONAL, color, glm::min(intesity, 1.0f), 
		glm::vec3(0, 0, 0), dir, 0);
	return entity;
}
entt::entity Scene::CreateSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float intesity, glm::vec3 color)
{
	auto entity = CreateSceneObject("Spot Light");
	registry.emplace<CLight>(entity, LightType::SPOT, color, glm::min(intesity, 1.0f), pos, dir, cutoff);
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

entt::entity Scene::CreateModelObject(const std::string& nodePath, const std::string& elePath, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	//from path get the part after the last "/" and before "."
	auto name = nodePath.substr(nodePath.find_last_of("/\\") + 1);
	name = name.substr(0, name.find_last_of("."));
	auto entity = CreateSceneObject(name);

	CTriMesh mesh;
	mesh.InitializeFrom(nodePath, elePath);
	registry.emplace<CTriMesh>(entity, mesh);
	auto& transform = registry.emplace<CTransform>(entity, position, rotation, scale);
	transform.SetPivot(mesh.GetBoundingBoxCenter());
	auto& material = registry.emplace<CPhongMaterial>(entity);
	
	registry.emplace<CRigidBody>(entity);
	registry.emplace<CBoxCollider>(entity, mesh.GetBoundingBoxMin(), mesh.GetBoundingBoxMax());

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


float CPhysicsBounds::MagImpulseCollistionFrom(float e, float m, glm::mat3 I, glm::vec3 v, glm::vec3 n, glm::vec3 r)
{
	//eq 5 from https://en.wikipedia.org/wiki/Collision_response
	return glm::dot((-1.0f - e) * v, n) / (1.0f/m + glm::dot(glm::inverse(I) * glm::cross(r,n), n));
}

bool CPhysicsBounds::IsCollidingWith(const CBoxCollider& collider, glm::vec3& normal, glm::vec3& contanctPoint, float& penetration)
{
	//Go over the vertices of the box collider and figure if any of them are outside the bounds
	//if there is a collision init the collision normal to point inward to the bounds
	normal = glm::vec3(0.0f);
	contanctPoint = glm::vec3(0.0f);
	int numCollisions = 0;
	glm::vec3 collidedBoundFace = glm::vec3(0.0f);
	float maxPenetration = 0;
	for (int i = 0; i < 8; i++)
	{
		const glm::vec3 v = collider.vertices[i];
		if (v.x < min.x)
		{
			normal.x += 1.0f;
			if(glm::abs(v.x - min.x) > maxPenetration)
				contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(min.x, 0.0f, 0.0f);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.x - min.x));
		}
		else if (v.x > max.x)
		{
			normal.x += -1.0f;
			if (glm::abs(v.x - max.x) > maxPenetration)
				contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(max.x, 0.0f, 0.0f);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.x - max.x));
		}
		if (v.y < min.y)
		{
			normal.y += 1.0f;
			if (glm::abs(v.y - min.y) > maxPenetration)
				contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(0, min.y, 0);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.y - min.y));
		}
		else if (v.y > max.y)
		{
			normal.y += -1.0f;
			if (glm::abs(v.y - max.y) > maxPenetration)
				contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(0, max.y, 0);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.y - max.y));
		}
		if (v.z < min.z)
		{
			normal.z += 1.0f;
			if (glm::abs(v.z - min.z) > maxPenetration)
				contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(0, 0, min.z);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.z - min.z));
		}
		else if (v.z > max.z)
		{
			normal.z += -1.0f;
			if (glm::abs(v.z - max.z) > maxPenetration)
			contanctPoint = v;
			numCollisions++;
			collidedBoundFace += glm::vec3(0, 0, max.z);
			maxPenetration = glm::max(maxPenetration, glm::abs(v.z - max.z));
		}
	}
	normal = glm::normalize(normal);
	//contanctPoint = contanctPoint;
	collidedBoundFace = collidedBoundFace/ ((float)numCollisions);
	penetration = glm::dot(contanctPoint - collidedBoundFace, normal);
	return glm::length(normal) > 0.0f;
}

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, std::string path)
{
	imgMaps.insert({ slot, ImageMap(path, slot) });
	
	//TODO::Schedule texture synch with renderer
	scheduledTextureUpdate = true;
}

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, glm::uvec2 dims, 
	ImageMap::RenderImageMode mode, Camera camera)
{
	imgMaps.insert({ slot, ImageMap(dims, slot, mode, camera) });

	//TODO::Schedule texture synch with renderer
	scheduledTextureUpdate = true;
}

void CImageMaps::AddImageMap(ImageMap::BindingSlot slot, std::string path[6])
{
	imgMaps.insert({ slot, ImageMap(path, slot) });
	scheduledTextureUpdate = true;
}

void CImageMaps::RemoveImageMap(ImageMap::BindingSlot slot)
{
	imgMaps.erase(slot);
	
	//TODO::Schedule texture synch with renderer
	scheduledTextureUpdate = true;
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
	case ImageMap::BindingSlot::DISPLACEMENT:
		return "Displacement Map";
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

bool CBoxCollider::CollidingWith(CBoxCollider* other)
{
	return false;//TODO
}