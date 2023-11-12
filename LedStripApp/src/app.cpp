#include "app.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <tchar.h>
#include <string>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

App::App()
{
    hwnd = nullptr;
    windowClass = {};
    numFramesInFlight = 3;
    frameContext = new FrameContext[numFramesInFlight];
    frameIndex = 0;
    numBackBuffers = 3;
    pd3dDevice = nullptr;
    pd3dRtvDescHeap = nullptr;
    pd3dSrvDescHeap = nullptr;
    pd3dCommandQueue = nullptr;
    pd3dCommandList = nullptr;
    fence = nullptr;
    fenceEvent = nullptr;
    fenceLastSignaledValue = 0;
    pSwapChain = nullptr;
    hSwapChainWaitableObject = nullptr;
    mainRenderTargetResource = new ID3D12Resource * [numBackBuffers];
    mainRenderTargetDescriptor = new D3D12_CPU_DESCRIPTOR_HANDLE[numBackBuffers];
}

void App::Run()
{
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin("App Window", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
        ImGui::Text("Text");
        ImGui::End();
        ImGui::PopStyleVar(1);

        // Rendering
        ImGui::Render();

        FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        pd3dCommandList->ClearRenderTargetView(mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        pd3dCommandList->OMSetRenderTargets(1, &mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        pd3dCommandList->SetDescriptorHeaps(1, &pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        pd3dCommandList->ResourceBarrier(1, &barrier);
        pd3dCommandList->Close();

        pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&pd3dCommandList);

        pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = fenceLastSignaledValue + 1;
        pd3dCommandQueue->Signal(fence, fenceValue);
        fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();
}

bool App::Init()
{
    InitWindow();
    if (!InitD3D())
    {
        return false;
    }
    InitImgui();
}

void App::InitWindow()
{
    // Create application window
    windowClass = { sizeof(windowClass), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Led Strip App", nullptr };
    RegisterClassExW(&windowClass);
    hwnd = CreateWindowW(windowClass.lpszClassName, L"Led Strip Controller", WS_OVERLAPPEDWINDOW, 50, 50, 400, 400, nullptr, nullptr, windowClass.hInstance, this);
}

bool App::InitD3D()
{
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }
}

void App::InitImgui()
{
    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(pd3dDevice, numFramesInFlight,
        DXGI_FORMAT_R8G8B8A8_UNORM, pd3dSrvDescHeap,
        pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
}

bool App::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = numBackBuffers;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = numBackBuffers;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < numBackBuffers; i++)
        {
            mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < numFramesInFlight; i++)
        if (pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&pd3dCommandList)) != S_OK ||
        pd3dCommandList->Close() != S_OK)
        return false;

    if (pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)) != S_OK)
        return false;

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        pSwapChain->SetMaximumFrameLatency(numBackBuffers);
        hSwapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void App::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain) { pSwapChain->SetFullscreenState(false, nullptr); pSwapChain->Release(); pSwapChain = nullptr; }
    if (hSwapChainWaitableObject != nullptr) { CloseHandle(hSwapChainWaitableObject); }
    for (UINT i = 0; i < numFramesInFlight; i++)
        if (frameContext[i].CommandAllocator) { frameContext[i].CommandAllocator->Release(); frameContext[i].CommandAllocator = nullptr; }
    if (pd3dCommandQueue) { pd3dCommandQueue->Release(); pd3dCommandQueue = nullptr; }
    if (pd3dCommandList) { pd3dCommandList->Release(); pd3dCommandList = nullptr; }
    if (pd3dRtvDescHeap) { pd3dRtvDescHeap->Release(); pd3dRtvDescHeap = nullptr; }
    if (pd3dSrvDescHeap) { pd3dSrvDescHeap->Release(); pd3dSrvDescHeap = nullptr; }
    if (fence) { fence->Release(); fence = nullptr; }
    if (fenceEvent) { CloseHandle(fenceEvent); fenceEvent = nullptr; }
    if (pd3dDevice) { pd3dDevice->Release(); pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void App::CreateRenderTarget()
{
    for (UINT i = 0; i < numBackBuffers; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, mainRenderTargetDescriptor[i]);
        mainRenderTargetResource[i] = pBackBuffer;
    }
}

void App::CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < numBackBuffers; i++)
        if (mainRenderTargetResource[i]) { mainRenderTargetResource[i]->Release(); mainRenderTargetResource[i] = nullptr; }
}

void App::WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &frameContext[frameIndex % numFramesInFlight];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (fence->GetCompletedValue() >= fenceValue)
        return;

    fence->SetEventOnCompletion(fenceValue, fenceEvent);
    WaitForSingleObject(fenceEvent, INFINITE);
}

FrameContext* App::WaitForNextFrameResources()
{
    UINT nextFrameIndex = frameIndex + 1;
    frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &frameContext[nextFrameIndex % numFramesInFlight];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        waitableObjects[1] = fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

LRESULT CALLBACK WINAPI App::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    // Disable ALT application menu
    if (msg == WM_SYSCOMMAND)
    {
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        return ::DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    if (msg == WM_DESTROY)
    {
        ::PostQuitMessage(0);
        return 0;
    }

    App* pThis;

    if (msg == WM_NCCREATE)
    {
        pThis = static_cast<App*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

        SetLastError(0);
        if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
        {
            if (GetLastError() != 0)
                return FALSE;
        }
    }
    else 
    {
        pThis = reinterpret_cast<App*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        if (msg == WM_SIZE)
        {
            if (pThis->pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                pThis->WaitForLastSubmittedFrame();
                pThis->CleanupRenderTarget();
                HRESULT result = pThis->pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
                assert(SUCCEEDED(result) && "Failed to resize swapchain.");
                pThis->CreateRenderTarget();
            }
            return 0;
        }
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

App::~App()
{
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
}