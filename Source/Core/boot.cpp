#include "Core.h"
#include "../Rendering/Model.h"
#include "Camera.h"

int main(void)
{
	Core* core = new Core();

	//core->objects3d.push_back(new Model("models/cube.obj", glm::vec3(5.0f, 0.0f, 0.0f)));
	//core->objects3d.push_back(new Model("models/scene.obj", "textures/uvmap.DDS", glm::vec3(0.0f, 0.0f, 0.0f), 0.001f));
	core->objects3d.push_back(new Model("models/suzanne.obj", "textures/uvmap.DDS", glm::vec3(2.5f, 0.0f, 0.0f), 1.0f));
	core->objects3d.push_back(new Model("models/suzanne.obj", "textures/uvmap.DDS", glm::vec3(-2.5f, 0.0f, 0.0f), 1.0f));
	core->objects3d.push_back(new Model("models/suzanne.obj", "textures/uvmap.DDS", glm::vec3(0, 0, 0), 1.0f));

	core->Cameras.push_back(new Camera(glm::vec3(0.0f, 0.0f, 5.0f), core->window));

	core->MainLoop();
	delete core;

	return 0;
}

