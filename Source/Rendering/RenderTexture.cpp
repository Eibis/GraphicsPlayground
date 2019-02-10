#include "RenderTexture.h"

#include <stdio.h>

#include <GL/glew.h>

#include <string>
#include <cstring>
#include "../Shaders/shader.hpp"
#include "../Core/Core.h"

using namespace std;

RenderTexture::RenderTexture()
{
	Init();
}


RenderTexture::~RenderTexture()
{
}

void RenderTexture::Init()
{
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// The depth buffer
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		fprintf(stderr, "Failed to initialize Framebuffer\n");

		return;
	}
	// The fullscreen quad's FBO
	GLuint quad_VertexArrayID;
	glGenVertexArrays(1, &quad_VertexArrayID);
	glBindVertexArray(quad_VertexArrayID);

	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	/* hardcoded test */
	string v_shader("Source/Shaders/Passthrough.vertexshader");
	string f_shader("Source/Shaders/SimpleTexture.fragmentshader");
	/**/

	if (Core::Instance->shaders.count(v_shader) == 0)
	{
		QuadProgramID = LoadShaders(v_shader, f_shader);
		Core::Instance->shaders.insert_or_assign(v_shader, QuadProgramID);
	}
	else
	{
		QuadProgramID = Core::Instance->shaders[v_shader];
	}

	GLuint texID = glGetUniformLocation(QuadProgramID, "renderedTexture");
	GLuint timeID = glGetUniformLocation(QuadProgramID, "time");
}

void RenderTexture::SetAsTarget()
{
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void RenderTexture::Draw()
{
	
}