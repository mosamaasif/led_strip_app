#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

class Window
{
private:
	static const int m_numFramesInFlight = 3;
	static const int m_numBackBuffers = 3;
	HWND m_hwnd;
	WNDCLASSEXW m_windowClass;
	FrameContext m_frameContext[m_numFramesInFlight];
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
	ID3D12Resource* m_mainRenderTargetResource[m_numBackBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE  m_mainRenderTargetDescriptor[m_numBackBuffers];

public:
	Window();

	bool Init();
	void PreRender();
	void Render();
	bool IsOpen();
	void WaitForLastSubmittedFrame();
	inline HWND GetHandle() const { return m_hwnd; }
	inline ID3D12Device* GetDevice() const { return m_pd3dDevice; }
	inline int GetFramesInFlight() const { return m_numFramesInFlight; }
	inline ID3D12DescriptorHeap* GetSrvDescHeap() const { return m_pd3dSrvDescHeap; }

	~Window();

private:
	void InitWindow();
	bool InitD3D();
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	FrameContext* WaitForNextFrameResources();
	static LRESULT CALLBACK WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

