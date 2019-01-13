#include "Core.h"
#include "../Rendering/Model.h"

int main(void)
{
	Core* core = new Core();

	//core->objects3d.push_back(new Model("models/cube.obj", glm::vec3(5.0f, 0.0f, 0.0f)));
	core->objects3d.push_back(new Model("models/scene.obj", "textures/uvmap.DDS", glm::vec3(0.0f, 0.0f, 0.0f), 0.001f));
	//	core->objects3d.push_back(new Model("models/suzanne.obj", "textures/uvmap.DDS", glm::vec3(0.0f, 4.0f, 0.0f), 3.2f));

	core->MainLoop();
	delete core;

	return 0;
}

