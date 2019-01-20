#include "Utilities.h"
#include <stdio.h>
#include "glfw3.h"

Utilities* Utilities::Instance;

Utilities::Utilities()
{
	LastTime = glfwGetTime();
	NbFrames = 0;
}

Utilities::~Utilities()
{
}

void Utilities::UpdateFrameCount()
{
	// Measure speed
	double currentTime = glfwGetTime();
	NbFrames++;
	if (currentTime - LastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
		printf("%f ms/frame\n", 1000.0 / double(NbFrames));
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
