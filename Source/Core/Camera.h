#pragma once

#include "../Rendering/Model.h"

struct GLFWwindow;

struct SortStruct
{
	SortStruct()
	{}

	bool operator () (Model* i, Model* j)
	{
		return glm::distance(i->Position, CameraPosition) > glm::distance(j->Position, CameraPosition);
	}

public:
	glm::vec3 CameraPosition;
};

enum class FrustumPlane
{
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	NEAR,
	FAR
};

class Camera
{
public:
	Camera(glm::vec3 origin, GLFWwindow* window);
	~Camera();

	glm::mat4 ProjMatr;
	glm::mat4 ViewMatr;

	vector<Model*> RenderQueue;

	void Render(const vector<Model*>& objects);

	bool CheckIsInFrustum(Box* box);

private:
	
	void ComputeMatricesFromInputs(GLFWwindow* window);
	void ComputeCameraFrustum(const glm::mat4& m);
	
	GLFWwindow* Window;

	SortStruct SortAux;

	/* input */

	// Initial position : on +Z
	glm::vec3 CameraPosition;

	// Initial horizontal angle : toward -Z
	float horizontalAngle = 3.14f;
	// Initial vertical angle : none
	float verticalAngle = 0.0f;
	// Initial Field of View
	float initialFoV = 45.0f;

	float speed = 2.0f;
	float mouseSpeed = 0.0025f;

	void ComputeMatricesFromInputs();

	glm::vec4 CameraFrustum[6];

};