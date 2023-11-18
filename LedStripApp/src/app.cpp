#include "app.h"
#include "imgui.h"
#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <filesystem>

#pragma comment(lib, "shell32.lib")

App::App() : m_LedController(), m_Window() {}

bool App::Init()
{
	if (!m_Window.Init())
	{
		return false;
	}
    LoadSettings();
	return true;
}

void App::LoadSettings()
{
    std::wstring path = FetchSettingsPath();
    if (std::empty(path))
    {
        return;
    }

    std::ifstream file(path + L"\\settings.txt");
    if (!file.is_open())
    {
        SaveSettings();
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::string value = line.substr(line.find_first_of("=") + 1);
        if (line.find("name") != std::string::npos)
        {
           memcpy(m_LedController.name, value.c_str(), value.size());
        }
        else if (line.find("on") != std::string::npos)
        {
            m_LedController.SetDeviceOnFlag(std::stoi(value));
        }
        else if (line.find("color") != std::string::npos) 
        {
            std::istringstream ss(value);
            std::string colorVal;
            int i = 0;
            while (ss >> colorVal)
            {   
                m_LedController.color[i++] = std::stof(colorVal);
            }
        }
        else if (line.find("brightness") != std::string::npos)
        {
            m_LedController.brightness = std::stof(value);
        }
    }
    file.close();
}

void App::SaveSettings()
{
    std::wstring path = FetchSettingsPath();
    if (std::empty(path))
    {
        return;
    }

    if (!std::filesystem::exists(path))
    {
        std::filesystem::create_directory(path);
    }

    std::ofstream file(path + L"\\settings.txt", std::fstream::trunc);
    if (!file.is_open())
    {
        return;
    }
    file << "name=" << m_LedController.name << "\n";
    file << "on=" << m_LedController.IsDeviceOn() << "\n";
    file << "color=" << m_LedController.color[0] << " " << m_LedController.color[1] << " " << m_LedController.color[2] << "\n";
    file << "brightness=" << m_LedController.brightness << "\n";

    file.close();
}

std::wstring App::FetchSettingsPath()
{
    PWSTR path;
    HRESULT result = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &path);

    if (result != S_OK)
    {
        return std::wstring();
    }

    std::wstring strpath = std::wstring(path) + L"\\LedStripApp";
    CoTaskMemFree(path);
    return strpath;
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
            if (ImGui::InputText("Device Name", m_LedController.name, IM_ARRAYSIZE(m_LedController.name)));
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
    SaveSettings();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
