#pragma once

#include "window.h"
#include "blemanager.h"

class App
{
private:
	Window m_window;
	BLEManager m_ledManager;

public:
	App();

	bool Init();
	void Run();

	~App();

private:
	void InitImgui();
	void RenderUI();
};