#pragma once

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

// Include GLEW
#include <GL/glew.h>

#include <string>
#include <vector>
using namespace std;

class Camera;

class Box
{
public:

	Box(glm::vec3 origin)
	{
		Origin = origin;

		_mMinX = 0.0f;
		_mMaxX = 0.0f;

		_mMinY = 0.0f;
		_mMaxY = 0.0f;

		_mMinZ = 0.0f;
		_mMaxZ = 0.0f;
	}

	void UpdateBox(const glm::vec3& vertex)
	{
		if (vertex.x > _mMaxX)
			_mMaxX = vertex.x;
		if (vertex.x < _mMinX)
			_mMinX = vertex.x;

		if (vertex.y > _mMaxY)
			_mMaxY = vertex.y;
		if (vertex.y < _mMinY)
			_mMinY = vertex.y;

		if (vertex.z > _mMaxZ)
			_mMaxZ = vertex.z;
		if (vertex.z < _mMinZ)
			_mMinZ = vertex.z;
	}

	void ApplyWithScale(float scale)
	{
		Scale = scale;

		wMinX = _mMinX * Scale + Origin.x;
		wMaxX = _mMaxX * Scale + Origin.x;

		wMinY = _mMinY * Scale + Origin.y;
		wMaxY = _mMaxY * Scale + Origin.y;

		wMinZ = _mMinZ * Scale + Origin.z;
		wMaxZ = _mMaxZ * Scale + Origin.z;
	}

	/*world space*/
	float wMinX;
	float wMaxX;

	float wMinY;
	float wMaxY;

	float wMinZ;
	float wMaxZ;

	float Scale;

private:

	glm::vec3 Origin;

	/*model space*/
	float _mMinX;
	float _mMaxX;

	float _mMinY;
	float _mMaxY;

	float _mMinZ;
	float _mMaxZ;

};

class Model
{
public:
	Model(const string& path, const string& texture_path, glm::vec3 origin, float scale);
	Model(const string& path, const string& texture_path, glm::vec3 origin, float scale, const string& normalmap_path, const string& specularmap_path);
	~Model();

	GLuint Draw(Camera* camera, GLuint currentShaderID);
	
	glm::vec3 Position;

	Box* BoundingBox;

	bool IsTransparent = false;
	bool HasNormalMap = false;
	bool HasSpecularMap = false;

private:

	void Init(const string& path, const string& texture_path, glm::vec3 origin, float scale);

	string DiffusePath;
	string NormalMapPath;
	string SpecularPath;

	float Scale;

	glm::mat4 ModelMatrix;

	GLuint ShaderID;
	GLuint MVPID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint ModelMatrix3X3ID;

	GLuint DiffuseTextureID;
	GLuint NormalTextureID;
	GLuint SpecularTextureID;

	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint UVBuffer;
	GLuint TangentBuffer;
	GLuint BiTangentBuffer;
	GLuint ElementBuffer;

	GLuint DiffuseTexture;
	GLuint NormalTexture;
	GLuint SpecularTexture;

	std::vector< glm::vec3 > Vertices;
	std::vector< glm::vec2 > UVs;
	std::vector< glm::vec3 > Normals;
	std::vector< glm::vec3 > Tangents;
	std::vector< glm::vec3 > Bitangents;

	std::vector<unsigned int> Indices;

	void Load(const string& path, const string& texture_path, const string& normalmap_path, const string& specularmap_path);
	bool LoadOBJ(const string& path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals);
	void ComputeTangentBasis(std::vector<glm::vec3> & vertices, std::vector<glm::vec2> & uvs, std::vector<glm::vec3> & normals, std::vector<glm::vec3> & tangents, std::vector<glm::vec3> & bitangents);
};

