#include <string>

class Text2D
{

public:
	
	Text2D(const char * texturePath);
	~Text2D();

	void Load(const char * texturePath);
	void Draw();

	std::string Text; 
	int X;
	int Y; 
	int Size;

private:

	unsigned int Text2DTextureID;
	unsigned int Text2DVertexBufferID;
	unsigned int Text2DUVBufferID;
	unsigned int Text2DShaderID;
	unsigned int Text2DUniformID;
};