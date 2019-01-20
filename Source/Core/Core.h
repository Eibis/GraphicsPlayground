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
class Camera;
struct GLFWwindow;

class Core
{
public:
	static Core* Instance;

	Core();
	~Core();

	int MainLoop();

	void InitLights(GLuint programID);
	void DrawLights();

	vector<Camera*> Cameras;
	vector<Model*> objects3d;
	map<string, GLuint> shaders;

	GLuint VertexArrayID;
	GLFWwindow* window;
	
private:

	int Init();
	int Exit();

	/* lights */
	GLuint LightID;
};

