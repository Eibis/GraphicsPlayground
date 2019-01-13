#pragma once

#include <vector>
#include <string>
#include <map>

// Include GLEW
#include <GL/glew.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

using namespace std;

class Model;

class Core
{
public:
	static Core* Instance;

	Core();
	~Core();

	int MainLoop();

	void InitLights(GLuint programID);
	void DrawLights();

	vector<Model*> objects3d;
	map<string, GLuint> shaders;

	GLuint VertexArrayID;
	
	glm::mat4 ProjMatr;
	glm::mat4 ViewMatr;

private:

	int Init();
	int Exit();

	/* lights */
	GLuint LightID;

	/* input */
	
	// Initial position : on +Z
	glm::vec3 position = glm::vec3(0, 0, 5);
	// Initial horizontal angle : toward -Z
	float horizontalAngle = 3.14f;
	// Initial vertical angle : none
	float verticalAngle = 0.0f;
	// Initial Field of View
	float initialFoV = 45.0f;

	float speed = 3.0f; // 3 units / second
	float mouseSpeed = 0.005f;

	void ComputeMatricesFromInputs();

};

