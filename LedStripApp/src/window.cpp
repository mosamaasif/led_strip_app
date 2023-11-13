#include "window.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window()
{
    m_hwnd = nullptr;
    m_windowClass = {};
    m_frameIndex = 0;
    m_pd3dDevice = nullptr;
    m_pd3dRtvDescHeap = nullptr;
    m_pd3dSrvDescHeap = nullptr;
    m_pd3dCommandQueue = nullptr;
    m_pd3dCommandList = nullptr;
    m_fence = nullptr;
    m_fenceEvent = nullptr;
    m_fenceLastSignaledValue = 0;
    m_pSwapChain = nullptr;
    m_hSwapChainWaitableObject = nullptr;
}

bool Window::Init()
{
    InitWindow();
    if (!InitD3D())
    {
        return false;
    }
}

void Window::InitWindow()
{
    // Create application window
    m_windowClass = { sizeof(m_windowClass), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Led Strip App", nullptr };
    RegisterClassExW(&m_windowClass);
    m_hwnd = CreateWindowW(m_windowClass.lpszClassName, L"Led Strip Controller", WS_OVERLAPPEDWINDOW, 50, 50, 400, 400, nullptr, nullptr, m_windowClass.hInstance, this);
}

bool Window::InitD3D()
{
    // Initialize Direct3D
    if (!CreateDeviceD3D(m_hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(m_windowClass.lpszClassName, m_windowClass.hInstance);
        return false;
    }
}

bool Window::IsOpen()
{
    bool isOpen = true;
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            isOpen = false;
    }
    return isOpen;
}

void Window::PreRender()
{

}

void Window::Render()
{
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = m_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
    m_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    m_pd3dCommandList->ClearRenderTargetView(m_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
    m_pd3dCommandList->OMSetRenderTargets(1, &m_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    m_pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_pd3dCommandList->ResourceBarrier(1, &barrier);
    m_pd3dCommandList->Close();

    m_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_pd3dCommandList);

    m_pSwapChain->Present(1, 0); // Present with vsync
    UINT64 fenceValue = m_fenceLastSignaledValue + 1;
    m_pd3dCommandQueue->Signal(m_fence, fenceValue);
    m_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
}

bool Window::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = m_numBackBuffers;
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
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        m_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
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
        desc.NumDescriptors = m_numBackBuffers;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (m_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < m_numBackBuffers; i++)
        {
            m_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (m_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (m_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < m_numFramesInFlight; i++)
        if (m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&m_pd3dCommandList)) != S_OK ||
        m_pd3dCommandList->Close() != S_OK)
        return false;

    if (m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)) != S_OK)
        return false;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&m_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        m_pSwapChain->SetMaximumFrameLatency(m_numBackBuffers);
        m_hSwapChainWaitableObject = m_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void Window::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (m_pSwapChain) { m_pSwapChain->SetFullscreenState(false, nullptr); m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_hSwapChainWaitableObject != nullptr) { CloseHandle(m_hSwapChainWaitableObject); }
    for (UINT i = 0; i < m_numFramesInFlight; i++)
        if (m_frameContext[i].CommandAllocator) { m_frameContext[i].CommandAllocator->Release(); m_frameContext[i].CommandAllocator = nullptr; }
    if (m_pd3dCommandQueue) { m_pd3dCommandQueue->Release(); m_pd3dCommandQueue = nullptr; }
    if (m_pd3dCommandList) { m_pd3dCommandList->Release(); m_pd3dCommandList = nullptr; }
    if (m_pd3dRtvDescHeap) { m_pd3dRtvDescHeap->Release(); m_pd3dRtvDescHeap = nullptr; }
    if (m_pd3dSrvDescHeap) { m_pd3dSrvDescHeap->Release(); m_pd3dSrvDescHeap = nullptr; }
    if (m_fence) { m_fence->Release(); m_fence = nullptr; }
    if (m_fenceEvent) { CloseHandle(m_fenceEvent); m_fenceEvent = nullptr; }
    if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void Window::CreateRenderTarget()
{
    for (UINT i = 0; i < m_numBackBuffers; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, m_mainRenderTargetDescriptor[i]);
        m_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void Window::CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < m_numBackBuffers; i++)
        if (m_mainRenderTargetResource[i]) { m_mainRenderTargetResource[i]->Release(); m_mainRenderTargetResource[i] = nullptr; }
}

void Window::WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &m_frameContext[m_frameIndex % m_numFramesInFlight];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No m_fence was signaled

    frameCtx->FenceValue = 0;
    if (m_fence->GetCompletedValue() >= fenceValue)
        return;

    m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

FrameContext* Window::WaitForNextFrameResources()
{
    UINT nextFrameIndex = m_frameIndex + 1;
    m_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { m_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &m_frameContext[nextFrameIndex % m_numFramesInFlight];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no m_fence was signaled
    {
        frameCtx->FenceValue = 0;
        m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        waitableObjects[1] = m_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

LRESULT CALLBACK WINAPI Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        PostQuitMessage(0);
        return 0;
    }

    Window* pThis;

    if (msg == WM_NCCREATE)
    {
        pThis = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
        SetLastError(0);
        if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
        {
            if (GetLastError() != 0)
                return FALSE;
        }
    }
    else
    {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        if (msg == WM_SIZE)
        {
            if (pThis->m_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                pThis->WaitForLastSubmittedFrame();
                pThis->CleanupRenderTarget();
                HRESULT result = pThis->m_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
                assert(SUCCEEDED(result) && "Failed to resize swapchain.");
                pThis->CreateRenderTarget();
            }
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

Window::~Window()
{
    CleanupDeviceD3D();
    DestroyWindow(m_hwnd);
    UnregisterClassW(m_windowClass.lpszClassName, m_windowClass.hInstance);
}