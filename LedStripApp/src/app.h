#pragma once

#include "window.h"
#include "ledcontroller.h"

class App
{
private:
	Window m_Window;
	LEDController m_LedController;

public:
	App();

	bool Init();
	void Run();

	~App();

private:
	std::wstring FetchSettingsPath();
	void LoadSettings();
	void SaveSettings();
	void RenderUI();
};