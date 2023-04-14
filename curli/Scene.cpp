#include <Scene.h>
#include <GLFWHandler.h>
#include <Eigen/Eigenvalues>
#include <thread>
#include <filesystem>

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
	std::fill(this->vertexNormals.begin(), this->vertexNormals.end(), glm::vec3(0.0f));
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

uint64_t cantor(uint32_t x, uint32_t y) {
	return ((x + y) * (x + y + 1u)) / 2u + y;
}

void CTriMesh::GenerateFaceFrom(const glm::vec3 v0,const glm::vec3 v1,const glm::vec3 v2,
	const glm::vec3 vIn, const glm::ivec3 vertIndices)
{
	glm::ivec3 face;
	//compute the normal for the face
	const glm::vec3 e1 = v1 - v0;
	const glm::vec3 e2 = v2 - v0;
	glm::vec3 normal = glm::normalize(glm::cross(e2, e1));
	const glm::vec3 inwardVec = vIn - v0;
	if (glm::dot(normal, inwardVec) >= 0)
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
			if (glm::any(glm::isnan(normal)))
			{
				printf("normal is nan\n");
				normal = glm::vec3(0, 0, 0);
			}
			this->vertexNormals.push_back(normal);

			//Texture
			this->textureCoords.push_back({ 0,0 });//insert empty coords

			face[j] = this->vertices.size() - 1;
			//tetgen2Face.insert(std::make_pair(indices[j], face[j]));
		}
		else//existing vertex
		{
			this->vertexNormals[vertIndices[j]] += normal;// we will normalize in the end
			face[j] = vertIndices[j];
		}
	}
	this->faces.push_back(face);
}

// Computes the Z-order curve value of a given pair of (x, y) coordinates
unsigned int zOrder2D(unsigned int x, unsigned int y) {
	unsigned int z = 0;
	for (int i = 0; i < sizeof(unsigned int) * CHAR_BIT / 2; i++) {
		z |= (x & (1 << i)) << i | (y & (1 << i)) << (i + 1);
	}
	return z;
}

unsigned int zOrder3D(unsigned int x, unsigned int y, unsigned int z) {
	unsigned int code = 0;
	for (int i = 0; i < sizeof(unsigned int) * CHAR_BIT / 3; i++) {
		code |= (x & (1 << i)) << (2 * i) | (y & (1 << i)) << (2 * i + 1) | (z & (1 << i)) << (2 * i + 2);
	}
	return code;
}

struct TripleHash {
	size_t hash(const glm::ivec3& t) const {
		std::array<size_t, 3> a = { (size_t)t.x, (size_t)t.y, (size_t)t.z };
		std::sort(a.begin(), a.end());
		return cantor(cantor(a[0], a[1]), a[2]);
	}
	size_t operator()(const glm::ivec3& t) const {
		return hash(t);
	}
	bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
		return hash(a) == hash(b);
	}
};


// Hash function for unordered_map that uses the Z-order curve value
struct PairHash {
	size_t hash(const glm::ivec2& t) const {
		std::array<size_t, 2> a = { (size_t)t.x, (size_t)t.y };
		std::sort(a.begin(), a.end());
		return cantor(a[0], a[1]);
	}
	size_t operator()(const glm::ivec2& p) const {
		return hash(p);
	}
	bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
		return hash(a) == hash(b);
	}
};

namespace fs = std::filesystem;

void CTriMesh::InitializeFrom(const std::string& nodePath, const std::string elePath,
	std::vector<Spring>& springs, Eigen::VectorXf& nodes, std::unordered_map<int, int>& volIdx2SurfIdx)
{	
	printf("==========Reading ele & node files==========\n");
	// Make sure files exist and good
	assert(fs::exists(nodePath) && fs::path(nodePath).extension() == ".node" && "Invalid extension for tetgen files");
	assert(fs::exists(elePath) && fs::path(elePath).extension() == ".ele" && "Invalid extension for tetgen files");

	// Read node file
	std::ifstream nodeFile(nodePath, std::ios::in);
	int numNodes, numDims, numAttrs, hasBdry, firstNodeIdx = std::numeric_limits<int>::max();
	nodeFile >> numNodes >> numDims >> numAttrs >> hasBdry;
	int nodeIdx = 0;
	nodes.resize(numNodes * 3);
	Eigen::AlignedBox3f bbox;
	while (!nodeFile.eof() && nodeIdx < numNodes) {
		int idx, bdry;
		nodeFile >> idx;
		firstNodeIdx = std::min(firstNodeIdx, idx);
		for (int i = 0; i < 3; i++) {
			nodeFile >> nodes[nodeIdx * 3 + i];
			bbox.extend(Eigen::Vector3f(nodes[nodeIdx * 3 + i], nodes[nodeIdx * 3 + i], nodes[nodeIdx * 3 + i]));
		}
		if (hasBdry) {
			nodeFile >> bdry;
		}
		nodeIdx += 1;
	}

	//go over all the nodes and normalize them to -5 5 range
	float maxDim = std::max(bbox.sizes().x(), std::max(bbox.sizes().y(), bbox.sizes().z()));
	float scale = 10.0f / maxDim;
	for (int i = 0; i < numNodes; i++) {
		for (int j = 0; j < 3; j++) {
			nodes[i * 3 + j] = (nodes[i * 3 + j] - bbox.center()[j]) * scale;
		}
	}

	// Read ele file and isolate surface mesh
	std::unordered_map<glm::ivec3, int, TripleHash, TripleHash> surfFaceIdx;
	std::unordered_map<glm::ivec3, int, TripleHash, TripleHash> face2InVertIdx;
	std::unordered_set<glm::ivec2, PairHash, PairHash> edgeIdxs;
	std::ifstream eleFile(elePath, std::ios::in);
	int numTets, numVertsPerTet, hasAttrs;
	eleFile >> numTets >> numVertsPerTet >> hasAttrs;
	int nTet = 0;
	while (!eleFile.eof() && nTet++ < numTets) {
		int idx;
		eleFile >> idx;
		glm::ivec4 tet;
		eleFile >> tet.x >> tet.y >> tet.z >> tet.w;
		tet -= glm::ivec4(firstNodeIdx);

		// Add edges to surface face set
		for (glm::ivec4 tetFace : std::array<glm::ivec4, 4>{ { {0, 1, 3, 2}, { 1,2,3,0 }, { 0,3,2,1 }, { 0,1,2,3 }}}) {
			glm::ivec3 face(tet[tetFace.x], tet[tetFace.y], tet[tetFace.z]);
			if (!surfFaceIdx.count(face)) {
				surfFaceIdx.emplace(face, 0);
			}
			surfFaceIdx[face] += 1;
			face2InVertIdx.emplace(face, tet[tetFace.w]);
		}
		//printf("[%d/%d tet] (%d surface faces) <%d,%d,%d,%d>\n", idx, numTets, surfFaceIdx.size(), tet.x, tet.y, tet.z, tet.w);

		// Add edges to set
		/*const std::array<std::pair<size_t, size_t>, 6> = {
			std::make_pair<size_t, size_t>()
		};*/
		for (glm::ivec2 tetEdge : std::array<glm::ivec2, 6> { { {0, 1}, { 1,2 }, { 0,2 }, { 0,3 }, { 1,3 }, { 2,3 }}}) {
			edgeIdxs.emplace(tet[tetEdge.x], tet[tetEdge.y]);
		}
	}

	// Indices of vertices on the surface of the mesh
	std::unordered_set<int> exteriorVertexIndices;
	int m = 0;
	for (const auto [face, count] : surfFaceIdx) {
		if (count == 2) {
			m += 1;
		}
		if (count != 1) continue;
		for (int i = 0; i < 3; i++) {
			exteriorVertexIndices.insert(face[i]);
		}
	}
	printf("# interior faces = %d, exterior faces = %d\n", m, surfFaceIdx.size() - m);

	// Map volume node index to surface vertex index
	for (int idx : exteriorVertexIndices) {
		volIdx2SurfIdx.emplace(idx, this->vertices.size());
		this->vertices.push_back(glm::make_vec3(nodes.segment<3>(idx * 3).data()));
		this->textureCoords.push_back({ 0.0f, 0.0f });
	}

	// Create surface triangles
	this->vertexNormals.resize(this->vertices.size(), glm::vec3(0.0f));
	for (const auto [face, count] : surfFaceIdx) {
		if (count != 1) continue;
		const glm::ivec3 f(volIdx2SurfIdx[face[0]], volIdx2SurfIdx[face[1]], volIdx2SurfIdx[face[2]]);
		this->faces.push_back(f);

		// Compute normal for face
		const glm::vec3 e1 = this->vertices[f[1]] - this->vertices[f[0]];
		const glm::vec3 e2 = this->vertices[f[2]] - this->vertices[f[0]];
		glm::vec3 normal = glm::normalize(glm::cross(e2, e1));
		const glm::vec3 inwardVec = glm::make_vec3(nodes.segment<3>(face2InVertIdx.at(face) * 3).data()) -
			this->vertices[f[0]];
		if (glm::dot(normal, inwardVec) >= 0) {
			normal *= -1.0f;
		}
		assert(!glm::any(glm::isnan(normal)));

		for (int i = 0; i < 3; i++) {
			this->vertexNormals[f[i]] += normal;
		}
	}


	// Normalize vertex normals
	for (auto& n : this->vertexNormals)
		n = glm::normalize(n);

	// Create springs
	for (const glm::ivec2 &edge : edgeIdxs) {
		Eigen::Vector3f a = nodes.segment<3>(edge.x * 3);
		Eigen::Vector3f b = nodes.segment<3>(edge.y * 3);
		springs.emplace_back(edge, (a - b).norm());
	}
	printf("==========Done==========\n\ttets: %d\n\t#verts: %d\n\t#normals: %d\n\t#faces: %d\n\t#springs:%d #nodes:%d\n",
		numTets, this->GetNumVertices(), this->GetNumNormals(), this->GetNumFaces(),
		springs.size(), nodes.size() / 3);
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

void CSoftBody::UpdateStiffnessMatrix() {

	// Loop over all springs
	for (const Spring& spring : springs) {
		// Indices of the two nodes connected by the spring
		const int i = spring.nodes[0];
		const int j = spring.nodes[1];

		// Positions of the two nodes
		const Eigen::Vector3f& pi = nodePositions.segment<3>(i * 3);
		const Eigen::Vector3f& pj = nodePositions.segment<3>(j * 3);
		const float curLenght = (pj - pi).norm();
		
		const Eigen::Matrix3f Kij = spring.k * (-Eigen::Matrix3f::Identity() + spring.restLength / curLenght *
			(Eigen::Matrix3f::Identity() - (pj - pi) * (pj - pi).transpose() / (pj - pi).squaredNorm()));

		// Fill in the matrix entries corresponding to the i-th node and jth node using double for loops
		for (size_t ii = 0; ii < 3; ii++)
		{
			for (size_t jj = 0; jj < 3; jj++)
			{
				stiffnessMatrix.coeffRef(i * 3 + ii, j * 3 + jj) = Kij(ii, jj);
				stiffnessMatrix.coeffRef(i * 3 + jj, j * 3 + ii) = -Kij(ii, jj);
			}

		}
	}
}

void CSoftBody::UpdateMassMatrix()
{
	massMatrix = Eigen::SparseMatrix<float>(nodePositions.size(), nodePositions.size());
	massMatrix.reserve(Eigen::VectorXi::Constant(nodePositions.size(), 6 * 9));

	for (int i = 0; i < nodePositions.size(); i++)
		massMatrix.insert(i, i) = glm::max(massPerNode, 0.01f);
	massMatrix.makeCompressed();
}

void CSoftBody::SetSpringKs(float k)
{
	k = glm::max(k, 0.001f);
	//parallel for each using cpp set all springs.k values to k
	std::for_each(springs.begin(), springs.end(), [k](Spring& spring) {
		spring.k = k;
		});
}

void CSoftBody::ApplyImpulse(Eigen::Vector3f imp, int nodeIdx)
{
	nodeExtForces.segment<3>(nodeIdx * 3) += imp / massPerNode;
	//nodeVelocities.segment<3>(nodeIdx * 3) += imp / massPerNode;
}

void CSoftBody::UpdateNodeForces(const Eigen::VectorXf& nodePos)
{
	// Calculate internal forces
	nodeTotalForces.setZero(nodePos.size());
	for (Spring& spring : springs)
	{
		const Eigen::Vector3f& node0 = nodePos.segment<3>(spring.nodes[0] * 3);
		const Eigen::Vector3f& node1 = nodePos.segment<3>(spring.nodes[1] * 3);
		auto force = spring.CalculateForce(node0, node1);
		/*force += -spring.damping * (nodeVelocities.segment<3>(spring.nodes[0] * 3) -
			nodeVelocities.segment<3>(spring.nodes[1] * 3));*/
		nodeTotalForces.segment<3>(spring.nodes[0] * 3) += force;
		nodeTotalForces.segment<3>(spring.nodes[1] * 3) -= force;
	}

	// Add gravitational force
	const Eigen::Vector3f gravity(0, 0, -gravity * 1.f);
	for (int i = 0; i < nodePos.size(); i += 3)
	{
		nodeTotalForces.segment<3>(i) += glm::max(massPerNode, 0.01f) * gravity * 200.f;
	}

	// Add external forces
	nodeTotalForces = nodeTotalForces + nodeExtForces;

}


void CSoftBody::TakeFwEulerStep(float dt)
{
	UpdateNodeForces(nodePositions);
	
	UpdateStiffnessMatrix();
	// Solve for v_{t+1} where (M - dt*dt *K) * vv_{t+1} = M * v_{t} + dt * f_{t}
	float engDiffSq;
	const Eigen::VectorXf prevMomentum = massMatrix * nodeVelocities;
	Eigen::SparseMatrix<float> MminusdtsK = massMatrix - dt * dt * stiffnessMatrix;
	Eigen::BiCGSTAB<Eigen::SparseMatrix<float>> solver;
	solver.compute(MminusdtsK);
	
	if (solver.info() != Eigen::Success)
	{
		std::cerr << "Failed to decompose matrix." << std::endl;
		return;
	}
	
	if (nodeTotalForces.norm() < 0.0001f)
	{
		//printf("Forces are too small\n");
		return;
	}
	int counter = 100;
	do
	{
		nodeVelocities = solver.solve(massMatrix * nodeVelocities + dt * nodeTotalForces);
		if (solver.info() != Eigen::Success)
		{
			std::cerr << "Failed to solve linear system." << std::endl;
			return;
		}
		
		UpdateNodeForces(nodePositions + nodeVelocities * dt);
		engDiffSq = (massMatrix * nodeVelocities - prevMomentum - dt * nodeTotalForces).squaredNorm();
		//prevMomentum = massMatrix * nodeVelocities;
		
	} while (engDiffSq > 0.01f && counter-- > 0);
	//check if nodeVelocities are all zero
	if (nodeVelocities.any() > 0.001f)
	{
		dirty = true;
	}
	
	nodePositions += nodeVelocities * dt;
	nodeExtForces.setZero();
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
	std::vector<Spring> springs;
	Eigen::VectorXf nodePositions;
	std::unordered_map<int, int> volIdx2SurfIdx;
	mesh.InitializeFrom(nodePath, elePath, springs, nodePositions, volIdx2SurfIdx);
	registry.emplace<CTriMesh>(entity, mesh);
	auto& transform = registry.emplace<CTransform>(entity, position, rotation, scale);
	transform.SetPivot(mesh.GetBoundingBoxCenter());
	auto& material = registry.emplace<CPhongMaterial>(entity);
	
	registry.emplace<CSoftBody>(entity, springs, nodePositions, volIdx2SurfIdx);
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

void CSoftBody::Update()
{
}
