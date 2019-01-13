#pragma once

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

// Include GLEW
#include <GL/glew.h>

#include <string>
#include <vector>
using namespace std;

class Model
{
public:
	Model(const string& path, const string& texture_path, glm::vec3 origin, float scale = 1.0f);
	~Model();

	GLuint Draw(GLuint currentShaderID);

private:

	float Scale;

	glm::mat4 ModelMatrix;

	GLuint ShaderID;
	GLuint MVPID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint TextureID;

	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint UVBuffer;
	GLuint ElementBuffer;
	GLuint Texture;

	std::vector< glm::vec3 > Vertices;
	std::vector< glm::vec2 > UVs;
	std::vector< glm::vec3 > Normals;

	std::vector<unsigned int> Indices;

	void Load(const string& path, const string& texture_path);
	bool LoadOBJ(const string& path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals);
	GLuint LoadDDS(const char * imagepath);
};

