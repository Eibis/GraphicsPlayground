#pragma once

class Text2D;

class Utilities
{
public:
	Utilities();
	~Utilities();

	void UpdateFrameCount(Text2D* text2D);

	static Utilities* GetInstance();
private:
	double LastTime = 0;
	int NbFrames = 0;

	static Utilities* Instance;
};

