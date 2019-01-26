#include "Utilities.h"
#include <stdio.h>
#include "glfw3.h"
#include "../Rendering/text2D.hpp"

Utilities* Utilities::Instance;

Utilities::Utilities()
{
	LastTime = glfwGetTime();
	NbFrames = 0;
}

Utilities::~Utilities()
{
}

void Utilities::UpdateFrameCount(Text2D* text2D)
{
	// Measure speed
	double currentTime = glfwGetTime();
	NbFrames++;
	if (currentTime - LastTime >= 1.0) 
	{ 
		text2D->Text = std::to_string(1000.0 / double(NbFrames)) + " ms/frame\n";

		NbFrames = 0;
		LastTime += 1.0;
	}
}

Utilities* Utilities::GetInstance()
{
	if (Instance == nullptr)
		Instance = new Utilities();

	return Instance;
}
