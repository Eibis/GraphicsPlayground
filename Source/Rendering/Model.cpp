#include "Model.h"
#include <string>
#include <cstring>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

#include "../Core/Core.h"
#include "../Shaders/shader.hpp"
#include "../Core/Camera.h"
#include "vboindexer.hpp"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

using namespace std;

Model::Model(const string& path, const string& texture_path, glm::vec3 origin, float scale)
{
	Init(path, texture_path, origin, scale);
}

Model::Model(const string& path, const string& texture_path, glm::vec3 origin, float scale, const string& normalmap_path, const string& specularmap_path)
{
	NormalMapPath = normalmap_path;
	SpecularPath = specularmap_path;

	HasNormalMap = true;
	HasSpecularMap = true;
	
	Init(path, texture_path, origin, scale);
}

void Model::Init(const string& path, const string& texture_path, glm::vec3 origin, float scale)
{
	DiffusePath = texture_path;

	Position = origin;
	Scale = scale;

	BoundingBox = new Box(Position);

	Load(path, DiffusePath, NormalMapPath, SpecularPath);

	glm::mat4 scaled_mat = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
	ModelMatrix = glm::translate(scaled_mat, origin);
}

Model::~Model()
{
	delete BoundingBox;

	glDeleteBuffers(1, &VertexBuffer);
	glDeleteBuffers(1, &UVBuffer);
	glDeleteBuffers(1, &NormalBuffer);
	glDeleteBuffers(1, &ElementBuffer);
	glDeleteBuffers(1, &TangentBuffer);
	glDeleteBuffers(1, &BiTangentBuffer);

	glDeleteTextures(1, &DiffuseTexture);
	glDeleteTextures(1, &NormalTexture);
	glDeleteTextures(1, &SpecularTexture);
}

GLuint Model::Draw(Camera* camera, GLuint currentShaderID)
{
	bool init_shader = currentShaderID != ShaderID;
	if(init_shader)
		glUseProgram(ShaderID);

	glm::mat4 mv = camera->ViewMatr * ModelMatrix;
	glm::mat4 mvp = camera->ProjMatr * mv;

	glUniformMatrix4fv(MVPID, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

	if(HasNormalMap)
	{
		glm::mat3 mv3x3 = glm::mat3(mv);
		glUniformMatrix3fv(ModelMatrix3X3ID, 1, GL_FALSE, &mv3x3[0][0]);
	}

	if (init_shader)
	{
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &camera->ViewMatr[0][0]);

		Core::Instance->DrawLights();

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
		glUniform1i(DiffuseTextureID, 0);

		if(HasNormalMap)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NormalTexture);
			glUniform1i(NormalTextureID, 1);
		}

		if(HasSpecularMap)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, SpecularTexture);
			glUniform1i(SpecularTextureID, 2);
		}
	}

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	if (HasNormalMap)
	{
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (HasNormalMap)
	{
		glBindBuffer(GL_ARRAY_BUFFER, TangentBuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, BiTangentBuffer);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);

	if (IsTransparent)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		Indices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	if (IsTransparent)
		glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if (HasNormalMap)
	{
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
	}

	return ShaderID;
}

void Model::Load(const string& path, const string& texture_path, const string& normalmap_path, const string& specularmap_path)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool res = LoadOBJ(path, vertices, uvs, normals);

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	ComputeTangentBasis(
		vertices, uvs, normals, // input
		tangents, bitangents    // output
	);

	if (!HasNormalMap)
		indexVBO(vertices, uvs, normals, Indices, Vertices, UVs, Normals);
	else
		indexVBO_TBN(vertices, uvs, normals, tangents, bitangents, Indices, Vertices, UVs, Normals, Tangents, Bitangents);

	vertices.clear();
	uvs.clear();
	normals.clear();
	tangents.clear();
	bitangents.clear();
	
	string v_shader, f_shader;

	/* hardcoded test */
	if (!HasNormalMap)
	{
		v_shader = "Source/Shaders/StandardShading.vertexshader";
		f_shader = "Source/Shaders/StandardShading.fragmentshader";
	}
	else
	{
		v_shader = "Source/Shaders/NormalMapping.vertexshader";
		f_shader = "Source/Shaders/NormalMapping.fragmentshader";
	}
	/**/

	if (Core::Instance->shaders.count(v_shader) == 0)
	{
		ShaderID = LoadShaders(v_shader, f_shader);
		Core::Instance->shaders.insert_or_assign(v_shader, ShaderID);

		Core::Instance->InitLights(ShaderID);
	}
	else
	{
		ShaderID = Core::Instance->shaders[v_shader];
	}

	MVPID = glGetUniformLocation(ShaderID, "MVP");
	ViewMatrixID = glGetUniformLocation(ShaderID, "V");
	ModelMatrixID = glGetUniformLocation(ShaderID, "M");

	if (HasNormalMap)
	{
		ModelMatrix3X3ID = glGetUniformLocation(ShaderID, "MV3x3");
	}
	
	DiffuseTextureID = glGetUniformLocation(ShaderID, "DiffuseTextureSampler");
	NormalTextureID = glGetUniformLocation(ShaderID, "NormalTextureSampler");
	SpecularTextureID = glGetUniformLocation(ShaderID, "SpecularTextureSampler");

	DiffuseTexture = LoadDDS(DiffusePath.c_str());

	if(HasNormalMap)
		NormalTexture = LoadBMP_custom(NormalMapPath.c_str());

	if (HasSpecularMap)
		SpecularTexture = LoadDDS(SpecularPath.c_str());

	glBindVertexArray(Core::Instance->VertexArrayID);

	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(glm::vec3), &Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &UVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &NormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(glm::vec3), &Normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(unsigned int), &Indices[0], GL_STATIC_DRAW);

	if (HasNormalMap)
	{
		glGenBuffers(1, &TangentBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, TangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, Tangents.size() * sizeof(glm::vec3), &Tangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &BiTangentBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, BiTangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, Bitangents.size() * sizeof(glm::vec3), &Bitangents[0], GL_STATIC_DRAW);
	}
}

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool Model::LoadOBJ(
	const string& path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) 
{
	printf("Loading OBJ file %s...\n", path.c_str());

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file = fopen(path.c_str(), "r");

	if (file == NULL) 
	{
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) 
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) 
		{
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) 
		{
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) 
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else 
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) 
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		BoundingBox->UpdateBox(vertex);

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}

	BoundingBox->ApplyWithScale(Scale);

	fclose(file);
	return true;
}

void Model::ComputeTangentBasis(
	// inputs
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
) 
{
	for (unsigned int i = 0; i < vertices.size(); i += 3) 
	{
		// Shortcuts for vertices
		glm::vec3 & v0 = vertices[i + 0];
		glm::vec3 & v1 = vertices[i + 1];
		glm::vec3 & v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2 & uv0 = uvs[i + 0];
		glm::vec2 & uv1 = uvs[i + 1];
		glm::vec2 & uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}
	
	for (unsigned int i = 0; i < vertices.size(); i += 1)
	{
		glm::vec3 & n = normals[i];
		glm::vec3 & t = tangents[i];
		glm::vec3 & b = bitangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f) {
			t = t * -1.0f;
		}
	}
}