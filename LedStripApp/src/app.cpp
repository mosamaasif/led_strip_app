#include "app.h"
#include "imgui.h"

App::App() : m_LedController(), m_Window() 
{
    m_LedController.ScanAndConnect();
}

bool App::Init()
{
	if (!m_Window.Init())
	{
		return false;
	}
	return true;
}

void App::RenderUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("App Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    // drawing the ui elements on the windows
    ImGui::Text(m_LedController.ConnectionStatusStr());
    if (!m_LedController.IsScanning()) {
        if (!m_LedController.IsConnected())
        {
            if (ImGui::Button("Connect"))
            {
                m_LedController.ScanAndConnect();
            }
        }
        else
        {
            if (ImGui::Button(m_LedController.IsDeviceOn() ? "Off" : "On"))
            {
                m_LedController.ToggleDevice();
            }
            if (ImGui::ColorEdit3("Color", m_LedController.color))
            {
                m_LedController.UpdateColor();
            }
            if (ImGui::SliderFloat("Brightness", &m_LedController.brightness, 0, 1))
            {
                m_LedController.UpdateBrightness();
            }
        }
    }
    
    ImGui::End();
    ImGui::PopStyleVar(1);

    // Rendering
    ImGui::Render();
}

void App::Run()
{
    while (m_Window.IsOpen())
    {
        m_LedController.TryJoinScanningThread();
        RenderUI();
        m_Window.Render();
    }

    m_Window.WaitForLastSubmittedFrame();
}

App::~App() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
