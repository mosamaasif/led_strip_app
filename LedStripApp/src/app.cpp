#include "app.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

App::App() : m_ledManager(), m_window() {
    m_ledManager.ScanAndConnect();
}

bool App::Init()
{
	if (!m_window.Init())
	{
		return false;
	}
	InitImgui();
	return true;
}

void App::InitImgui()
{
    // Show the window
    ShowWindow(m_window.GetHandle(), SW_SHOWDEFAULT);
    UpdateWindow(m_window.GetHandle());

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_window.GetHandle());
    ImGui_ImplDX12_Init(m_window.GetDevice(), m_window.GetFramesInFlight(),
        DXGI_FORMAT_R8G8B8A8_UNORM, m_window.GetSrvDescHeap(),
        m_window.GetSrvDescHeap()->GetCPUDescriptorHandleForHeapStart(),
        m_window.GetSrvDescHeap()->GetGPUDescriptorHandleForHeapStart());
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
    if (ImGui::Button("Scan and Connect"))
    {
        if (!m_ledManager.IsScanning())
        {
            m_ledManager.ScanAndConnect();
        }
    }
    if (m_ledManager.IsConnected() && ImGui::Button(m_ledManager.IsDeviceOn() ? "Off" : "On"))
    {
        m_ledManager.SetDeviceOn();
    }
    if (ImGui::ColorEdit3("clear color", m_ledManager.color))
    {
        m_ledManager.UpdateLedColor();
    }
    ImGui::Text(m_ledManager.ConnectionStatusStr());
    ImGui::End();
    ImGui::PopStyleVar(1);

    // Rendering
    ImGui::Render();
}

void App::Run()
{
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    while (m_window.IsOpen())
    {
        if (!m_ledManager.IsScanning())
        {
            m_ledManager.JoinScanningThread();
        }
        RenderUI();
        m_window.Render();
    }

    m_window.WaitForLastSubmittedFrame();
}

App::~App() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
