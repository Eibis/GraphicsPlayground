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
	Position = origin;
	Scale = scale;

	BoundingBox = new Box(Position);

	Load(path, texture_path);

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
	glDeleteTextures(1, &Texture);
}

GLuint Model::Draw(Camera* camera, GLuint currentShaderID)
{
	bool init_shader = currentShaderID != ShaderID;
	if(init_shader)
		glUseProgram(ShaderID);
	
	glm::mat4 mvp = camera->ProjMatr * camera->ViewMatr * ModelMatrix;

	glUniformMatrix4fv(MVPID, 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

	if (init_shader)
	{
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &camera->ViewMatr[0][0]);

		Core::Instance->DrawLights();

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
	}

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, NormalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		Indices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	return ShaderID;
}

void Model::Load(const string& path, const string& texture_path)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool res = LoadOBJ(path, vertices, uvs, normals);
	indexVBO(vertices, uvs, normals, Indices, Vertices, UVs, Normals);
	
	vertices.clear();
	uvs.clear();
	normals.clear();

	/* hardcoded test */
	string v_shader("Source/Shaders/StandardShading.vertexshader");
	string f_shader("Source/Shaders/StandardShading.fragmentshader");
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
	TextureID = glGetUniformLocation(ShaderID, "myTextureSampler");

	Texture = LoadDDS(texture_path.c_str());

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
