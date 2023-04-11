#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cyTriMesh.h>
#include <CyToGLMHelper.h>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/log_base.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include <lodepng.h>

#include <fstream>
#include <stdarg.h>
#include <string>
#include <unordered_map>
#include <tuple>
#include <execution>

//define a macro that takes a function calls it and ctaches opengl errors
#define GL_CALL(func) \
do { \
    func; \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        const char* errorString = (const char*)glad_glGetString(GL_VERSION); \
        const char* description = (const char*)glfwGetError(NULL); \
        printf("OpenGL error %d (%s) at %s:%d - %s\n", error, errorString, __FILE__, __LINE__, description); \
    } \
} while (false)

struct Camera
{
public:
	Camera(glm::vec3 center = { 0.f,0.f,0.f }, glm::vec3 eye = { 10.f,0.f,0.f }, glm::vec3 up={0.f, 1.f, 0.f},
		float fov = 45.f, float near = 0.1f, float far = 100.f,
		float aspectRatio = 1.f, bool perspective = true)
		:center(center), eye(eye), up(up),
		fov(fov), nearPlane(near), farPlane(far), 
		aspectRatio(aspectRatio), perspective(perspective)
	{
		this->CalculateViewMatrix();
		this->CalculateProjectionMatrix();
	}
	Camera(glm::vec3 center, glm::vec3 angles, float distance,
		float fov = 45.f, float near = 0.1f, float far = 100.f,
		float aspectRatio = 1.f, bool perspective = true)
		:center(center),
		fov(fov), nearPlane(near), farPlane(far),
		aspectRatio(aspectRatio), perspective(perspective)
	{
		SetOrbitAngles(angles);
		SetOrbitDistance(distance);
		this->CalculateViewMatrix();
		this->CalculateProjectionMatrix();
	}
	
	Camera(Camera& other)
		: center(other.center), eye(other.eye), up(other.up),
		fov(other.fov), nearPlane(other.nearPlane), farPlane(other.farPlane),
		aspectRatio(other.aspectRatio), perspective(other.perspective)
	{
		this->CalculateViewMatrix();
		this->CalculateProjectionMatrix();
	}


	static glm::mat3 tensorProduct(const glm::vec3& a, const glm::vec3& b) {
		glm::mat3 result;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				result[i][j] = a[i] * b[j];
			}
		}
		return result;
	}
	
	static Camera& Reflect(glm::vec3 point, glm::vec3 normal, Camera& camera)
	{
		glm::vec3 center = camera.GetCenter();
		glm::vec3 eye = camera.GetLookAtEye();
		glm::vec3 up = camera.GetLookAtUp();
		float fov = camera.GetFOV();
		float near = camera.GetNearPlane();
		float far = camera.GetFarPlane();
		float aspectRatio = camera.GetAspectRatio();
		bool perspective = camera.IsPerspective();
		normal = glm::normalize(normal);
		const glm::vec3 eyeToPoint = eye - point;
		const glm::vec3 centerToPoint = center - point;
		const glm::vec3 upToPoint = up - point;

		////reflect eye by the plane define by point and normal
		//const glm::vec3 reflectedEye = eye - 2.f * glm::dot(eyeToPoint, normal) * normal;
		////reflect center by the plane define by point and normal
		//const glm::vec3 reflectedCenter = center - 2.f * glm::dot(centerToPoint, normal) * normal;
		////reflect up by the plane define by point and normal
		//const glm::vec3 reflectedUp = up - 2.f * glm::dot(upToPoint, normal) * normal;
		//

		//return Camera(reflectedCenter, reflectedEye, glm::normalize(reflectedUp), 90, near, far, -1.0, perspective);

		//generate a transformation matrix that reflects the camera
		//1. move to the point
		//scale by -1 * normal
		//move back
		Camera copyCamera(camera);

		/*glm::mat3 t = glm::mat3(1.f) - 2.f * glm::outerProduct(normal, normal);

		glm::mat4 R(1.f);
		R[0] = glm::vec4(t[0], 0.f);
		R[1] = glm::vec4(t[1], 0.f);
		R[2] = glm::vec4(t[2], 0.f);

		glm::mat4 T(1.f);
		T[3] = glm::vec4(point, 1.0);

		R[3] = glm::vec4((glm::mat3(1.f) - t) * point, 1.f);*/
		
		/*copyCamera.viewMatrix = R * copyCamera.viewMatrix;*/
		
		return copyCamera;
		
	}
	
	glm::vec3 GetCenter() { return center; }
	
	glm::vec3 GetLookAtEye() { return eye; }
	glm::vec3 GetLookAtUp() { return up; }
	
	glm::vec3 GetOrbitAngles() 
	{ 
		return glm::vec3(
			glm::degrees(asin((eye.y - center.y) / glm::distance(eye, center))),
			glm::degrees(atan2f((-eye.z + center.z), (eye.x - center.x)))+90,
			0.f);
	}
	float GetOrbitDistance() { return glm::length(eye - center); }
	
	float GetFOV() { return fov; }
	float GetAspectRatio() { return aspectRatio; }
	float GetNearPlane() { return nearPlane; }
	float GetFarPlane() { return farPlane; }
	bool IsPerspective() { return perspective; }
	
	glm::mat4 GetViewMatrix()
	{
		if (viewDirty)
			CalculateViewMatrix();
		return viewMatrix;
	}
	glm::mat4 GetProjectionMatrix()
	{
		if (projectionDirty)
			CalculateProjectionMatrix();
		return projectionMatrix;
	}
	
	void inline SetCenter(glm::vec3 center, bool recalculate = false)
	{
		this->center = center;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	
	void inline SetLookAtUp(glm::vec3 up, bool recalculate = false)
	{
		this->up = up;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void inline SetLookAtEye(glm::vec3 eye, bool recalculate = false)
	{
		this->eye = eye;
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	

	void inline SetOrbitAngles(glm::vec3 angles, bool recalculate = false)
	{
		if (angles.x > 89.999f)
			angles.x = 89.999f;
		if (angles.x < -89.999f)
			angles.x = -89.999f;
		const float theta = glm::radians(angles.x);
		const float phi = glm::radians(angles.y);
		glm::vec3 unitSpherePos = {
			cos(theta) * sin(phi),
			sin(theta),
			cos(theta) * cos(phi)
		};
		eye = center + unitSpherePos * glm::length(eye-center);
		//up = {0.f, 1.f, 0.f};
		
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	void inline SetOrbitDistance(float distance, bool recalculate = false)
	{
		eye = center + glm::normalize(eye - center) * glm::max(distance, 0.001f);
		if (recalculate)
			CalculateViewMatrix();
		viewDirty = !recalculate;
	}
	
	//--------------------------------------------------------------------//
	void inline SetPerspective(bool perspective, bool recalculate = false)
	{
		this->perspective = perspective;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetFOV(float fov, bool recalculate = false)
	{
		this->fov = fov;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetAspectRatio(float aspectRatio, bool recalculate = false)
	{
		this->aspectRatio = aspectRatio;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetNearPlane(float nearPlane, bool recalculate = false)
	{
		this->nearPlane = nearPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	void inline SetFarPlane(float farPlane, bool recalculate = false)
	{
		this->farPlane = farPlane;
		if (recalculate)
			CalculateProjectionMatrix();
		projectionDirty = !recalculate;
	}
	
private:
	glm::vec3 eye = glm::vec3(1.f);
	glm::vec3 center = glm::vec3(0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 angles;
	
	float fov = 45.f;
	float aspectRatio = 1.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	bool perspective = true;
	
	bool viewDirty = false;
	bool projectionDirty = false;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	
	void inline CalculateViewMatrix()
	{
		viewMatrix = glm::lookAt(eye, center, up);
		viewDirty = false;
	}
	void inline CalculateProjectionMatrix()
	{
		if (perspective)
			projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		else
			projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, nearPlane, farPlane);

		projectionDirty = false;
	}
};

struct SpringNode
{
	SpringNode(glm::vec3 position, float mass = 1.0f)
	{
		this->position = position;
		this->mass = mass;
	}
	
	glm::vec3 position;
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
	//glm::vec3 force;
	float mass = 1.0f;
	int faceIndex = -1;//-1 = non-boundary
};

struct Spring
{
	Spring(glm::ivec2 nodes, float restLength)
	{
		this->nodes = nodes;
		this->restLength = restLength;
	}
	Spring(glm::ivec2 nodes, glm::vec3 node0Pos, glm::vec3 node1Pos)
	{
		this->nodes = nodes;
		this->restLength = glm::length(node0Pos - node1Pos);
	}
	float restLength;
	float k = 1.0f;//stiffness
	float damping = 0.01f;
	glm::ivec2 nodes;

	glm::vec3 CalculateForce(SpringNode& node0, SpringNode& node1) const
	{
		glm::vec3 springVector = node0.position - node1.position;
		float currentLength = glm::length(springVector);
		springVector = glm::normalize(springVector);
		glm::vec3 springForce = -k * (currentLength - restLength) * springVector;
		glm::vec3 dampingForce = -damping * (node1.velocity - node0.velocity);
		return springForce + dampingForce;
	}
};

enum class CType{
	Transform, TriMesh, 
	PhongMaterial, ImageMaps,
	Light, EnvironmentMap,
	PhysicsBounds,VelocityField2D,
	ForceField2D, RigidBody,
	BoxCollider, SoftBody,
	Count
};
struct Component
{
	virtual void Update() = 0;
	CType GetType() { return this->type; }
protected:
	CType type;
	
};

struct CTransform : Component
{
public:
	CTransform(glm::vec3 position = glm::vec3(0.f), glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f))
		: position(position), rotation(rotation), scale(scale)
	{
		//pivot = position;
		CalculateModelMatrix();
		type = CType::Transform;
	}

	//getter and setters
	void SetPosition(glm::vec3 position, bool recalculate = false)
	{
		this->position = position;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetEulerRotation(glm::vec3 rotation, bool recalculate = false)
	{
		this->rotation = rotation;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetEulerRotation(glm::mat4 rotationMat, bool recalculate = false)
	{
		this->rotationMatrix = rotationMat;
		//set rotation using matrix
		if (rotationMat[0][0] == 1.0f)
		{
			this->rotation.x = atan2f(rotationMat[0][2], rotationMat[2][3]);
			this->rotation.y = 0;
			this->rotation.z = 0;

		}
		else if (rotationMat[0][0] == -1.0f)
		{
			this->rotation.x = atan2f(rotationMat[0][2], rotationMat[2][3]);
			this->rotation.y = 0;
			this->rotation.z = 0;
		}
		else
		{

			this->rotation.x = atan2(-rotationMat[2][0], rotationMat[0][0]);
			this->rotation.y = asin(rotationMat[1][0]);
			this->rotation.z = atan2(-rotationMat[1][2], rotationMat[1][1]);
		}
		
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetScale(glm::vec3 scale, bool recalculate = false)
	{
		this->scale = scale;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void SetPivot(glm::vec3 pivot, bool recalculate = false)
	{
		this->pivot = pivot;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Translate(glm::vec3 offset, bool recalculate = false)
	{
		position += offset;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Rotate(glm::vec3 angles, bool recalculate = false)
	{
		this->rotation += angles;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	void Scale(glm::vec3 multiplier, bool recalculate = false)
	{
		this->scale *= multiplier;
		if (recalculate)
			CalculateModelMatrix();
		modelDirty = !recalculate;
	}
	glm::vec3 GetPosition() { return position; }
	glm::vec3 GetPivot() { return pivot; }
	glm::vec3 GetRotation() { return rotation; }
	glm::mat4 GetRotationMatrix() { return glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z); }
	glm::vec3 GetScale() { return scale; }
	CTransform* GetParent() { return parent; }
	glm::mat4 GetModelMatrix()
	{
		if (modelDirty)
			CalculateModelMatrix();
		return modelMatrix;
	}
	void Reset()
	{
		position = glm::vec3(0.f);
		rotation = glm::vec3(0.f);
		scale = glm::vec3(1.f);
		pivot = glm::vec3(0.f);
		CalculateModelMatrix();
	}
	
	//parenting
	void SetParent(CTransform* parent)
	{
		if(!parent)
		{
			//find this in parent's children and remove
			for (int i = 0;  this->parent && i < this->parent->children.size(); i++)
			{
				if (this->parent->children[i] == this)
				{
					this->parent->children.erase(this->parent->children.begin() + i);
					break;
				}
			}
		}
		else
		{
			bool found = false;
			for (int i = 0; i < parent->children.size(); i++)
			{
				if (parent->children[i] == this)
				{
					found = true;
					break;
				}
			}
			if(!found)
				parent->children.push_back(this);
		}
		this->parent = parent;
		CalculateModelMatrix();
	}
	
	void CalculateModelMatrix()
	{
		modelMatrix = glm::mat4(1.f);
		modelMatrix = glm::translate(modelMatrix, position);
		if (useRotationMatrix)
			modelMatrix = modelMatrix * glm::mat4(rotationMatrix);
		else
			modelMatrix = modelMatrix * glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
		modelMatrix = glm::scale(modelMatrix, scale);
		modelMatrix = glm::translate(modelMatrix, -pivot);//everything happens with respect to pivot
		if (parent != nullptr)
			modelMatrix *= parent->GetModelMatrix();

		modelDirty = false;
		for (auto child : children)
		{
			child->CalculateModelMatrix();
		}
	}

	bool useRotationMatrix = false;
	
	void Update();
	std::string entityName;
private:
	glm::vec3 position = glm::vec3(0.f);
	glm::vec3 rotation = glm::vec3(0.f);
	glm::vec3 scale = glm::vec3(1.f);
	glm::vec3 pivot = glm::vec3(0.f);//rotation and scaling pivot point
	
	glm::mat3 rotationMatrix = glm::mat3(1.f);

	glm::mat4 modelMatrix = glm::mat4(1.f);
	bool modelDirty = false;
	
	CTransform* parent = nullptr;
	std::vector<CTransform*> children;

};

enum class ShadingMode
{
	PHONG, EDITOR
};
struct CTriMesh : Component
{
public:
	static constexpr CType type = CType::TriMesh;
	CTriMesh(cy::TriMesh& mesh)
	{
		InitializeFrom(mesh);
	}

	CTriMesh(const std::string& path)
	{
		LoadObj(path.c_str());
	}
	
	CTriMesh()
	{
	}
	
	//deep copy constructor
	CTriMesh(const CTriMesh& other)
	{
		this->vertices = other.vertices;
		this->vertexNormals = other.vertexNormals;
		this->textureCoords = other.textureCoords;
		this->faces = other.faces;
		this -> shading = other.shading;
		this->bBoxMax = other.bBoxMax;
		this->bBoxMin = other.bBoxMin;
		this->bBoxInitialized = other.bBoxInitialized;
		this->tessellationLevel = other.tessellationLevel;
		this->visible = other.visible;
	}
	
	unsigned int GetNumVertices() const { return vertices.size(); }
	unsigned int GetNumFaces() const { return faces.size(); }
	unsigned int GetNumNormals() const { return vertexNormals.size(); }
	unsigned int GetNumTextureVertices() const { return  textureCoords.size(); }
	
	glm::vec3 GetVertex(unsigned int index) const { return vertices.at(index); }
	glm::vec3 GetVNormal(unsigned int index) const { return vertexNormals.at(index);}
	glm::vec2 GetVTexture(unsigned int index) const { return textureCoords.at(index); }
	glm::uvec3 GetFace(unsigned int index) const { return faces.at(index); }
	
	glm::vec3& GetVertex(unsigned int index) { return vertices.at(index); }
	glm::vec3& GetVNormal(unsigned int index) { return vertexNormals.at(index); }
	glm::vec2& GetVTexture(unsigned int index) { return textureCoords.at(index); }
	glm::uvec3& GetFace(unsigned int index) { return faces.at(index); }
	

	void* GetVertexDataPtr() { return &vertices.front(); }
	void* GetNormalDataPtr() { return &vertexNormals.front(); }
	void* GetTextureDataPtr() { return &textureCoords.front();}
	void* GetFaceDataPtr() { return &faces.front(); }

	
	/*
	* Create opengl friendly single buffer indexed mesh from cy::TriMesh
	*/
	void InitializeFrom(cy::TriMesh& mesh);
	void InitializeFrom(const std::string& nodePath, const std::string elePath,
		std::vector<Spring>& springs, std::vector<SpringNode>& nodes);

	inline void ComputeBoundingBox()
	{
		for (auto vertex : vertices)
		{
			bBoxMax = glm::max(bBoxMax, vertex);
			bBoxMin = glm::min(bBoxMin, vertex);
		}
		bBoxInitialized = true;
	}

	inline void ComputeNormals()
	{
		vertexNormals.resize(vertices.size());
		for(auto face : faces)
		{
			glm::vec3 v1 = vertices[face.x];
			glm::vec3 v2 = vertices[face.y];
			glm::vec3 v3 = vertices[face.z];

			glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

			vertexNormals[face.x] += normal;
			vertexNormals[face.y] += normal;
			vertexNormals[face.z] += normal;
		}
		
		for (auto& normal : vertexNormals)
			normal = glm::normalize(normal);
	}
	
	inline glm::vec3 GetBoundingBoxMin()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return bBoxMin;
	}

	inline glm::vec3 GetBoundingBoxMax()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return bBoxMax;
	}

	inline glm::vec3 GetBoundingBoxCenter()
	{
		if (!bBoxInitialized)
			ComputeBoundingBox();
		return (bBoxMin + bBoxMax) * .5f;
	}

	inline void LoadObj(const std::string& path)
	{
		cy::TriMesh mesh;
		mesh.LoadFromFileObj(path.c_str());
		InitializeFrom(mesh);
		
		printf("Loaded model with %d vertices, vertex normals %d, texture vertices %d, and faces %d from %s\n",
			GetNumVertices(),
			GetNumNormals(),
			GetNumTextureVertices(),
			GetNumFaces(),
			path.c_str());
	}
	
	inline ShadingMode GetShadingMode() { return shading; }
	inline void SetShadingMode(ShadingMode mode) { shading = mode; }

	void Update();
	
	void Clear()
	{
		bBoxInitialized = false;
		vertices.clear();
		vertexNormals.clear();
		textureCoords.clear();
		faces.clear();
	}

	glm::vec3 ApproximateTheClosestPointTo(glm::vec3 point, int stride = 1);

	
	bool visible = true;
	int tessellationLevel = 1;//None
private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertexNormals;
	std::vector<glm::vec2> textureCoords;
	std::vector<glm::uvec3> faces;
	
	glm::vec3 bBoxMin = glm::vec3(FLT_MAX);
	glm::vec3 bBoxMax = glm::vec3(-FLT_MAX);
	bool bBoxInitialized = false;
	ShadingMode shading = ShadingMode::PHONG; //0 = phong-color, 1 = phong-texture, 2 = editor mode

	void GenerateFaceFrom(const glm::vec3 v0, const glm::vec3 v1, const glm::vec3 v2,
		const glm::vec3 vIn, const glm::ivec3 vertIndices);
};

struct CPhongMaterial : Component
{
public:
	static constexpr CType type = CType::PhongMaterial;
	glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.3f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(0.9f, 0.9f, 0.9f);
	float shininess = 450.f;

	void Update();
};

struct ImageMap
{
	enum class BindingSlot {
		T_AMBIENT, T_DIFFUSE, T_SPECULAR, NORMAL, DISPLACEMENT, ENV_MAP
	};
	enum class RenderImageMode {
		REFLECTION, SHADOW, CUSTOM, NONE
	};
public:
	ImageMap()
	{}
	
	ImageMap(std::string path, BindingSlot slot)
		:path(path), bindingSlot(slot)
	{
		std::vector<unsigned char> png;
		//load and decode
		unsigned error = lodepng::load_file(png, path);
		if (error)
		{
			printf("Texture constructor\n\tlodepng:load error %d - %s\n", error, lodepng_error_text(error));
			return;
		}
		unsigned int w, h;
		error = lodepng::decode(image, w, h, png);
		dims.x = w;
		dims.y = h;

		//if there's an error, display it
		if (error)
		{
			printf("Texture constructor\n\tlodepng:decoder error %d - %s\n", error, lodepng_error_text(error));
			return;
		}
	}
	
	ImageMap(std::string path[6], BindingSlot slot)//creates a flat list of 6 images
		:path(path[0]), bindingSlot(slot)
	{
		for (int i = 0; i < 6; i++)
		{
			std::vector<unsigned char> png;
			//load and decode
			unsigned error = lodepng::load_file(png, path[i]);
			if (error)
			{
				printf("Texture constructor\n\tlodepng:load error %d - %s\n", error, lodepng_error_text(error));
				return;
			}
			unsigned int w, h;
			std::vector<unsigned char> tmp;
			error = lodepng::decode(tmp, w, h, png);
			if (slot == BindingSlot::ENV_MAP)
			{
				dims.x = w;
				dims.y = h;
			}
			else
			{
				dims.x += w;
				dims.y += h;
			}

			//if there's an error, display it
			if (error)
			{
				printf("Texture constructor\n\tlodepng:decoder error %d - %s\n", error, lodepng_error_text(error));
				return;
			}
			image.insert(image.end(), tmp.begin(), tmp.end());
		}
	}

	ImageMap(glm::uvec2 dims, BindingSlot slot, RenderImageMode rmode, Camera camera = Camera())
		:renderedImageCamera(camera), dims(dims), bindingSlot(slot), mode(rmode)
	{
		camera.SetAspectRatio((float)dims.x / (float)dims.y);
		image.clear();
	}
	
	ImageMap(ImageMap const& other)
	{
		image = other.image;
		dims = other.dims;
		bindingSlot = other.bindingSlot;
		path = other.path;
		mode = other.mode;
		renderedImageCamera = other.renderedImageCamera;
		programRenderedTexIndex = other.programRenderedTexIndex;
	}

	bool SetCamera(Camera camera)
	{
		if (IsRenderedImage())
		{
			renderedImageCamera = camera;
			return true;
		}
		return false;
	}

	//getters
	std::string GetPath() { return path; }
	std::vector<unsigned char>& GetImage() { return image; }
	glm::uvec2 GetDims() { return dims; }
	BindingSlot GetBindingSlot() { return bindingSlot; }
	std::string GetSlotName();
	Camera& GetRenderedImageCamera() { return renderedImageCamera; }
	bool IsRenderedImage() { return mode != RenderImageMode::NONE; }
	unsigned int GetProgramRenderedTexIndex() { return programRenderedTexIndex; }
	RenderImageMode GetRenderImageMode() { return mode; }

	//setters
	void SetProgramRenderedTexIndex(unsigned int index) { programRenderedTexIndex = index; }

	GLuint glID; //Maybe find a better way...
	float dispMultiplier = 0;
private:
	std::string path;
	std::vector<unsigned char> image;
	glm::uvec2 dims;
	BindingSlot bindingSlot;

	Camera renderedImageCamera;
	RenderImageMode mode = RenderImageMode::NONE;
	unsigned int programRenderedTexIndex = 0;
};
struct CImageMaps : Component
{
public:
	static constexpr CType type = CType::ImageMaps;

	CImageMaps()
	{}

	void AddImageMap(ImageMap::BindingSlot slot, std::string path);
	void AddImageMap(ImageMap::BindingSlot slot, std::string path[6]);
	void AddImageMap(ImageMap::BindingSlot slot, glm::uvec2 dims,
		ImageMap::RenderImageMode mode, Camera camera = Camera());
	void RemoveImageMap(ImageMap::BindingSlot slot);
	void Update();

	unsigned int inline GetNumMaps() { return imgMaps.size(); }
	std::unordered_map<ImageMap::BindingSlot, ImageMap>::iterator mapsBegin() { return imgMaps.begin(); }
	std::unordered_map<ImageMap::BindingSlot, ImageMap>::iterator mapsEnd() { return imgMaps.end(); }
	ImageMap& GetImageMap(ImageMap::BindingSlot slot) { return imgMaps[slot]; }

	bool scheduledTextureUpdate = false;
private:
	std::unordered_map<ImageMap::BindingSlot, ImageMap> imgMaps;
};

enum class LightType
{
	POINT,
	DIRECTIONAL,
	SPOT
};
struct CLight : Component
{
public:
	static bool slotUsed[15];//for glsl shader locations
	static constexpr CType type = CType::Light;
	
	CLight(LightType ltype, glm::vec3 color, float intensity, glm::vec3 position,
		glm::vec3 direction, float cutoff)
		:lightType(ltype)
	{
		this->color = color;
		this->intensity = intensity;
		direction = glm::normalize(direction);
		//Point light constructor
		if (ltype == LightType::POINT)
		{
			this->position = position;
			//invalid values for safety
			this->direction = glm::vec3(NAN, NAN, NAN);
			this->cutoff = -1;
			for (int i = 0; i < 5; i++)
			{
				if (!CLight::slotUsed[i])
				{
					CLight::slotUsed[i] = true;
					this->slot = i;
					break;
				}
			}
			if (this->slot == -1)
			{
				CLight::slotUsed[0] = true;
				this->slot = 0;
			}
		}
		//Directional light constructor
		else if (ltype == LightType::DIRECTIONAL)
		{
			this->direction = direction;
			lightType = LightType::DIRECTIONAL;

			//invalid values for safety
			this->position = glm::vec3(NAN, NAN, NAN);
			this->cutoff = -1;
			for (int i = 0; i < 5; i++)
			{
				if (!CLight::slotUsed[i + 5])
				{
					CLight::slotUsed[i + 5] = true;
					this->slot = i;
					break;
				}
			}
			if (this->slot == -1)
			{
				CLight::slotUsed[5] = true;
				this->slot = 0;
			}

		}
		//Spot light constructor
		else if (ltype == LightType::SPOT)
		{
			this->position = position;
			this->direction = direction;

			this->cutoff = cutoff;

			for (int i = 0; i < 5; i++)
			{
				if (!CLight::slotUsed[i + 10])
				{
					CLight::slotUsed[i + 10] = true;
					this->slot = i;
					break;
				}
			}
			if (this->slot == -1)
			{
				CLight::slotUsed[10] = true;
				this->slot = 0;
			}
		}

	}

	~CLight()
	{
		
		if (lightType == LightType::DIRECTIONAL)
			this->slot = this->slot + 5;
		else if (lightType == LightType::SPOT)
			this->slot = this->slot + 10;
		
		CLight::slotUsed[this->slot] = false;
	}

	glm::mat4 CalculateShadowMatrix(int side = -1)
	{
		if (lightType == LightType::POINT)
		{
			switch (side)
			{
			case 0:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				break;
			case 1:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				break;
			case 2:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			case 3:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
				break;
			case 4:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				break;
			case 5:
				return glm::scale(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f), glm::vec3(glm::length(this->position / 1000.f))) *
					glm::lookAt(position, position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
				break;
			default:
				break;
			}
		}
		else if (lightType == LightType::DIRECTIONAL)
		{
			glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.01f, 50.f);
			glm::mat4 lightView = glm::lookAt(-glm::normalize(direction) * 38.f,
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
			return lightProjection * lightView;
		}
		else
		{
			glm::mat4 lightProjection = glm::scale(glm::perspective(cutoff, 1.0f, 0.10f, 1000.f), 
				glm::vec3(glm::length(this->position/1000.f)));//todo
			//glm::mat4 lightProjection = glm::scale(glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.01f, 50.f), glm::vec3(1/this->position.z));
			glm::mat4 lightView = glm::lookAt(this->position,
				this->position + glm::normalize(direction),
				glm::vec3(0.0f, 1.0f, 0.0f));
			return lightProjection * lightView;
		}
	}
	LightType GetLightType() { return lightType; }
	bool IsCastingShadows() { return castingShadow; }

	void SetCastingShadows(bool castShadow) { 
		this->scheduledTextureUpdate = true;
		this->castingShadow = castShadow; 
	}

	glm::vec3 color;
	float intensity;
	float cutoff;
	glm::vec3 direction;
	glm::vec3 position;
	bool show = false;

	void Update();

	bool scheduledTextureUpdate = false;
	GLuint glID;
	int slot = -1;
private:
	LightType lightType;
	bool castingShadow = false;
};

struct CSkyBox : Component
{
public:
	static constexpr CType type = CType::EnvironmentMap;
	CSkyBox(std::string path[6])
	{
		sides.resize(6);
		for (int i = 0; i < 6; i++)
		{
			sides[i] = ImageMap(path[i], ImageMap::BindingSlot::ENV_MAP);
		}
		
		dims = glm::uvec2(sides[0].GetDims());//TODO:: better way ??
	}
	
	std::vector<unsigned char> GetSideImagesFlat();
	glm::uvec2 GetDims() { return dims; }

	void Update();
	std::vector<ImageMap> sides;//order: x+, x-, y+, y-, z+, z-
private:
	glm::uvec2 dims;
};


enum class FieldPlane
{
	XY, YZ, XZ
};
struct CVelocityField2D : Component
{
public:
	static constexpr CType type = CType::VelocityField2D;
	void Update();
	//function pointer here
	std::function<glm::vec2(glm::vec2)> field;
	float scaling = 1.f;

	FieldPlane plane;

	CVelocityField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{
	}

	glm::vec3 At(glm::vec3 p);
};

struct CForceField2D : Component
{
public:
	static constexpr CType type = CType::ForceField2D;
	void Update();
	//function pointer here
	std::function<glm::vec2(glm::vec2)> field;
	float scaling = 1.f;
	FieldPlane plane;

	CForceField2D(std::function<glm::vec2(glm::vec2)> field, FieldPlane plane = FieldPlane::XY)
		:field(field), plane(plane)
	{
	}

	glm::vec3 At(glm::vec3 p);
};

struct CSoftBody : Component
{
	static constexpr CType type = CType::SoftBody;
	CSoftBody()
	{}
	
	CSoftBody(std::vector<Spring> springs, std::vector<SpringNode> nodes)
	{
		this->nodes = nodes;
		this->springs = springs;
	}

	void Update();
	void TakeFwEulerStep(float dt);
	
	Eigen::SparseMatrix<float> CalculateStiffnessMatrix();
	
	std::vector<Spring> springs;
	std::vector<SpringNode> nodes;
};

struct CRigidBody : Component
{
public:
	static constexpr CType type = CType::RigidBody;
	CRigidBody(float mass = 1.0f, 
		glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation= glm::vec3(0.0f),
		float drag = 0.0f, float gravity = 0.f)
	{
		this->mass = mass;
		this->position = position;
		this->drag = drag;
		this->gravity = gravity;
		SetRotation(rotation);
	}

	float mass = 0.0f;
	float drag = 0.0f;
	glm::vec3 position = glm::vec3(0, 0, 0);

	glm::vec3 linearMomentum = glm::vec3(0, 0, 0);
	glm::vec3 angularMomentum = glm::vec3(0, 0, 0);

	void Update();

	void initializeInertiaTensor(const CTriMesh* mesh, CTransform* transform);
	void SetMassMatrix();
	
	void TakeFwEulerStep(float dt);
	void ResetToRest()
	{
		linearMomentum = glm::vec3(0, 0, 0);
		angularMomentum = glm::vec3(0, 0, 0);
		
		position = glm::vec3(0, 0, 0);
	}

	inline glm::vec3 GetVelocity()
	{
		return linearMomentum / mass;
	}
	//w =  R * I_rest^{-1} * R^T * L
	inline glm::vec3 GetAngularVelocity()
	{
		return orientationMatrix * 
			glm::inverse(inertiaAtRest) *
			(glm::transpose(orientationMatrix) * angularMomentum);
	}
	inline glm::mat3 GetOrientationMatrix()
	{
		return orientationMatrix;
	}
	
	inline glm::mat3 GetInertiaTensor()
	{
		return orientationMatrix * inertiaAtRest * glm::transpose(orientationMatrix);
	}
	
	inline void SetRotation(glm::vec3 rotation)
	{
		orientationQuat = glm::quat(rotation);
		orientationMatrix = glm::toMat3(orientationQuat);
	}

	inline glm::mat3 GetInertiaTensorAtRest()
	{
		return inertiaAtRest;
	}
	
	

	void ApplyLinearImpulse(glm::vec3 imp);
	void ApplyAngularImpulse(glm::vec3 imp);
	
	float gravity;
private:
	glm::mat3 inertiaAtRest;
	
	glm::quat orientationQuat;
	glm::mat3 orientationMatrix;
	
	glm::mat3 invMassMatrix;
};

struct CBoxCollider : Component
{
public:
	static constexpr CType type = CType::BoxCollider;
	CBoxCollider(glm::vec3 min, glm::vec3 max, float elasticity = 1.0f)
	{
		this->min = glm::min(min, max);
		this->max = glm::max(min, max);
		this->elasticity = elasticity;

		//Generate vertices
		vertices[0] = max;
		vertices[1] = glm::vec3(max.x, max.y, min.z);
		vertices[2] = glm::vec3(max.x, min.y, min.z);
		vertices[3] = glm::vec3(max.x, min.y, max.z);
		vertices[4] = glm::vec3(min.x, min.y, max.z);
		vertices[5] = min;
		vertices[6] = glm::vec3(min.x, max.y, min.z);
		vertices[7] = glm::vec3(min.x, max.y, max.z);
	}
	
	CBoxCollider(std::vector<glm::vec3> geometry, float elasticity = 1.0f)
	{
		//loop &find min and max over geometry
		glm::vec3 min = geometry[0];
		glm::vec3 max = geometry[0];
		for (int i = 1; i < geometry.size(); i++)
		{
			min = glm::min(min, geometry[i]);
			max = glm::max(max, geometry[i]);
		}
		CBoxCollider(min, max, elasticity);
	}
	
	//bool MoveAndCollide(glm::vec3 motion) {};
	bool CollidingWith(CBoxCollider* Other);
	void Update() {};

	glm::vec3 GetMin() { return min; }
	glm::vec3 GetMax() { return max; }
	
	void SetMin(glm::vec3 min) { this->min = min; }
	void SetMax(glm::vec3 max) { this->max = max; }

	void SetBounds(glm::vec3 min, glm::vec3 max)
	{
		this->min = glm::min(min, max);
		this->max = glm::max(min, max);
		
		vertices[0] = max;
		vertices[1] = glm::vec3(max.x, max.y, min.z);
		vertices[2] = glm::vec3(max.x, min.y, min.z);
		vertices[3] = glm::vec3(max.x, min.y, max.z);
		vertices[4] = glm::vec3(min.x, min.y, max.z);
		vertices[5] = min;
		vertices[6] = glm::vec3(min.x, max.y, min.z);
		vertices[7] = glm::vec3(min.x, max.y, max.z);
	}

	float elasticity;
	glm::vec3 vertices[8];
private:
	glm::vec3 min;
	glm::vec3 max;
};

//Box to contrain rigid bodies with colliders
struct CPhysicsBounds : Component
{
public:
	static constexpr CType type = CType::PhysicsBounds;
	CPhysicsBounds(glm::vec3 min, glm::vec3 max)
	{
		this->min = glm::min(min, max);
		this->max = glm::max(min, max);
	}

	glm::vec3 GetMin() { return min; }
	glm::vec3 GetMax() { return max; }

	void SetMin(glm::vec3 min) { this->min = min; dirty = true; }
	void SetMax(glm::vec3 max) { this->max = max; dirty = true; }

	inline bool IsDirty() { return dirty; }
	bool IsCollidingWith(const CBoxCollider& collider, glm::vec3& normal, glm::vec3& contanctPoint, float& penetration);


	void Update();
	std::vector<glm::vec3> GenerateVertices();
	float MagImpulseCollistionFrom(float e, float m, glm::mat3 I, glm::vec3 v, glm::vec3 n, glm::vec3 r);

private:
	glm::vec3 min;
	glm::vec3 max;
	bool dirty = false;
};

class Scene
{
public:
	Scene();
	~Scene();

	entt::entity CreateSceneObject(std::string name);
	//void DestroyEntity(entt::entity entity);

	void Update();
	
	/*
	* Returns a reference to the specified component of the specified entity. 
	* Use empty entity to fetch the first entity with the specified component
	*/
	template <typename C>
	C& GetComponent(entt::entity entity)
	{
		return registry.view<C>().get<C>(entity);
	};

	/*
	* Reads an obj file and adds and entity to the sceneObjects with a transform, phong material and a mesh
	*/
	entt::entity CreateModelObject(const std::string& meshPath, glm::vec3 position = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));
	/*
	* Reads a node and ele file to create a mesh with rigidbody, collider, transform, phong material and mesh
	*/
	entt::entity CreateModelObject(const std::string& nodePath, const std::string& elePath, glm::vec3 position = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));
	/*
	* Adds and entity to the sceneObjects with a transform, phong material and a mesh
	*/
	entt::entity CreateModelObject(cy::TriMesh& mesh, glm::vec3 position = glm::vec3(0.f),
		glm::vec3 rotation = glm::vec3(0.f), glm::vec3 scale = glm::vec3(1.f));
	/*
	* Creates a Point light source
	*/
	entt::entity CreatePointLight(glm::vec3 pos, float intesity,
		glm::vec3 color = glm::vec3(1, 1, 1));

	/*
	* Creates a Directional light source
	*/
	entt::entity CreateDirectionalLight(glm::vec3 dir, float intesity, glm::vec3 color);

	/*
	* Creates a Spot light source
	*/
	entt::entity Scene::CreateSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float intesity, glm::vec3 color);
	
	/*
	* Creates a bounding box that acts as a physics bound alongside with a VAO so it can be drawn
	*/
	entt::entity CreateBoundingBox(glm::vec3 min, glm::vec3 max, GLuint prograID = -1);

	/*
	* Inserts the registered entity to sceneObjects map with given name or given name + number returns identifier name
	*/
	inline std::string InsertSceneObject(std::string name, entt::entity entity)
	{
		auto it = sceneObjects.find(name);
		if (it != sceneObjects.end())
		{
			int i = 1;
			while (true)
			{
				it = sceneObjects.find(name + std::to_string(i));
				if (it == sceneObjects.end())
				{
					sceneObjects[name + std::to_string(i)] = entity;
					return name + std::to_string(i);
				}
				i++;
			}
		}
		else
			sceneObjects[name] = entity;
		return name;
	}
	/*
	* Gets the entity with the specified name
	*/
	inline entt::entity GetSceneObject(std::string name)
	{
		return sceneObjects.find(name) == sceneObjects.end() ? entt::tombstone : sceneObjects[name];
	}

	/*
	* Removes the enitity from the sceneObjects list and registry
	*/
	inline bool RemoveSceneObject(std::string name)
	{
		auto entity = sceneObjects[name];
		sceneObjects.erase(name);
		registry.destroy(entity);
		return entity != entt::tombstone;
	}

	inline bool RemoveSceneObject(entt::entity object)
	{
		//search sceneObjects for object
		for (auto it = sceneObjects.begin(); it != sceneObjects.end(); it++)
		{
			if (it->second == object)
			{
				sceneObjects.erase(it);
				registry.destroy(object);
				return true;
			}
		}
		return object != entt::tombstone;
	}

	inline std::unordered_map<std::string, entt::entity>::iterator sceneObjectsBegin()
	{
		return sceneObjects.begin(); 
	}

	inline std::unordered_map<std::string, entt::entity>::iterator sceneObjectsEnd()
	{
		return sceneObjects.end();
	}
	
	/*
	* Returns true if the entity has the given component
	*/
	bool EntityHas(entt::entity e, CType component);

	Camera camera;
	entt::registry registry;
	bool explicit_euler = true;
private:
	std::unordered_map<std::string, entt::entity> sceneObjects;
};