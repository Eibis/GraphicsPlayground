#pragma once
class Utilities
{
public:
	Utilities();
	~Utilities();

	void UpdateFrameCount();

	static Utilities* GetInstance();
private:
	double LastTime = 0;
	int NbFrames = 0;

	static Utilities* Instance;
};

