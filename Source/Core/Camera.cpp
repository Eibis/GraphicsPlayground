#include "Camera.h"
#include <algorithm>    // std::sort

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

using namespace std;

Camera::Camera(glm::vec3 origin, GLFWwindow* window)
{
	CameraPosition = origin;
	Window = window;
}

Camera::~Camera()
{
}

void Camera::Render(const vector<Model*>& objects)
{
	ComputeMatricesFromInputs();
	ComputeCameraFrustum(ProjMatr * ViewMatr);

	GLuint shader_id = 0;

	RenderQueue.clear();

	for (auto it = objects.begin(); it != objects.end(); ++it)
	{
		if(CheckIsInFrustum((*it)->BoundingBox))
			RenderQueue.push_back(*it);
	}

	SortAux.CameraPosition = CameraPosition;
	std::sort(RenderQueue.begin(), RenderQueue.end(), SortAux);

	for (auto it = RenderQueue.begin(); it != RenderQueue.end(); ++it)
	{
		shader_id = (*it)->Draw(this, shader_id);
	}
}

void Camera::ComputeMatricesFromInputs()
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(Window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(Window, 1024 / 2, 768 / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	// Move forward
	if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
		CameraPosition += direction * deltaTime * speed;
	// Move backward
	if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
		CameraPosition -= direction * deltaTime * speed;
	// Strafe right
	if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
		CameraPosition += right * deltaTime * speed;
	// Strafe left
	if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
		CameraPosition -= right * deltaTime * speed;
	// Strafe up
	if (glfwGetKey(Window, GLFW_KEY_Q) == GLFW_PRESS)
		CameraPosition += up * deltaTime * speed;
	// Strafe down
	if (glfwGetKey(Window, GLFW_KEY_E) == GLFW_PRESS)
		CameraPosition -= up * deltaTime * speed;

	float FoV = initialFoV;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjMatr = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatr = glm::lookAt(
		CameraPosition,           // Camera is here
		CameraPosition + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void Camera::ComputeCameraFrustum(const glm::mat4& m)
{
	CameraFrustum[(int)FrustumPlane::RIGHT].x = m[0][3] + m[0][0];
	CameraFrustum[(int)FrustumPlane::RIGHT].y = m[1][3] + m[1][0];
	CameraFrustum[(int)FrustumPlane::RIGHT].z = m[2][3] + m[2][0];
	CameraFrustum[(int)FrustumPlane::RIGHT].w = m[3][3] + m[3][0];

	CameraFrustum[(int)FrustumPlane::LEFT].x = m[0][3] - m[0][0];
	CameraFrustum[(int)FrustumPlane::LEFT].y = m[1][3] - m[1][0];
	CameraFrustum[(int)FrustumPlane::LEFT].z = m[2][3] - m[2][0];
	CameraFrustum[(int)FrustumPlane::LEFT].w = m[3][3] - m[3][0];

	CameraFrustum[(int)FrustumPlane::TOP].x = m[0][3] - m[0][1];
	CameraFrustum[(int)FrustumPlane::TOP].y = m[1][3] - m[1][1];
	CameraFrustum[(int)FrustumPlane::TOP].z = m[2][3] - m[2][1];
	CameraFrustum[(int)FrustumPlane::TOP].w = m[3][3] - m[3][1];

	CameraFrustum[(int)FrustumPlane::BOTTOM].x = m[0][3] + m[0][1];
	CameraFrustum[(int)FrustumPlane::BOTTOM].y = m[1][3] + m[1][1];
	CameraFrustum[(int)FrustumPlane::BOTTOM].z = m[2][3] + m[2][1];
	CameraFrustum[(int)FrustumPlane::BOTTOM].w = m[3][3] + m[3][1];

	CameraFrustum[(int)FrustumPlane::FAR].x = m[0][2];
	CameraFrustum[(int)FrustumPlane::FAR].y = m[1][2];
	CameraFrustum[(int)FrustumPlane::FAR].z = m[2][2];
	CameraFrustum[(int)FrustumPlane::FAR].w = m[3][2];

	CameraFrustum[(int)FrustumPlane::NEAR].x = m[0][3] - m[0][2];
	CameraFrustum[(int)FrustumPlane::NEAR].y = m[1][3] - m[1][2];
	CameraFrustum[(int)FrustumPlane::NEAR].z = m[2][3] - m[2][2];
	CameraFrustum[(int)FrustumPlane::NEAR].w = m[3][3] - m[3][2];
}

bool Camera::CheckIsInFrustum(Box* box)
{
	for (int i = 0; i < 6; i++)
	{
		int out = 0;

		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMinX, box->wMinY, box->wMinZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMaxX, box->wMinY, box->wMinZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMinX, box->wMaxY, box->wMinZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMaxX, box->wMaxY, box->wMinZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMinX, box->wMinY, box->wMaxZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMaxX, box->wMinY, box->wMaxZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMinX, box->wMaxY, box->wMaxZ, 1.0f)) < 0.0) ? 1 : 0);
		out += ((glm::dot(CameraFrustum[i], glm::vec4(box->wMaxX, box->wMaxY, box->wMaxZ, 1.0f)) < 0.0) ? 1 : 0);
		
		if (out == 8) 
			return false;
	}

	return true;
}