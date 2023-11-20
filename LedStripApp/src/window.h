#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

class Window
{
private:
	static const int NUM_FRAMES_IN_FLIGHT = 3;
	static const int NUM_BACK_BUFFERS = 3;
	HWND m_hwnd;
	WNDCLASSEXW m_windowClass;
	FrameContext m_frameContext[NUM_FRAMES_IN_FLIGHT];
	UINT m_frameIndex;
	ID3D12Device* m_pd3dDevice;
	ID3D12DescriptorHeap* m_pd3dRtvDescHeap;
	ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	ID3D12Fence* m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceLastSignaledValue;
	IDXGISwapChain3* m_pSwapChain;
	HANDLE m_hSwapChainWaitableObject;
	ID3D12Resource* m_mainRenderTargetResource[NUM_BACK_BUFFERS];
	D3D12_CPU_DESCRIPTOR_HANDLE  m_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];
	bool m_IsOpen;

public:
	Window();

	bool init();
	void render();
	inline bool isOpen() const { return m_IsOpen; }
	void waitForLastSubmittedFrame();

	~Window();

private:
	bool initWindow();
	bool initD3D();
	bool initImGui();
	bool createDeviceD3D(HWND hWnd);
	void createRenderTarget();
	FrameContext* waitForNextFrameResources();
	void cleanupDeviceD3D();
	void cleanupRenderTarget();
	void handleWindowMessages();
	static LRESULT CALLBACK WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

