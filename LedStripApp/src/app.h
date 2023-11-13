#pragma once

#include "window.h"
#include "ledstrip.h"

class App
{
private:
	Window m_window;
	LedStrip m_ledStrip;

public:
	App();

	bool Init();
	void Run();

	~App();

private:
	const char* OnScanAndConnectClicked();
	void ChangeLedStripColor(const float* color);

	void InitImgui();
	void RenderUI();
};