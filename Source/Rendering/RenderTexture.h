#pragma once

// Include GLEW
#include <GL/glew.h>

class RenderTexture
{
public:
	RenderTexture();
	~RenderTexture();

	void SetAsTarget();
	void Draw();

private: 

	GLuint FramebufferName = 0;
	GLuint QuadProgramID;

	void Init();
};

