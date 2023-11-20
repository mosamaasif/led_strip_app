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

	bool init();
	void run();

	~App();

private:
	std::wstring fetchSettingsPath();
	void loadSettings();
	void saveSettings();
	void renderUI();
};