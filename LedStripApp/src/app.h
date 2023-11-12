#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

class App
{
public:
	HWND hwnd;
	WNDCLASSEXW windowClass;
	int numFramesInFlight;
	FrameContext* frameContext;
	UINT frameIndex;
	int numBackBuffers;
	ID3D12Device* pd3dDevice;
	ID3D12DescriptorHeap* pd3dRtvDescHeap;
	ID3D12DescriptorHeap* pd3dSrvDescHeap;
	ID3D12CommandQueue* pd3dCommandQueue;
	ID3D12GraphicsCommandList* pd3dCommandList;
	ID3D12Fence* fence;
	HANDLE fenceEvent;
	UINT64 fenceLastSignaledValue;
	IDXGISwapChain3* pSwapChain;
	HANDLE hSwapChainWaitableObject;
	ID3D12Resource** mainRenderTargetResource;
	D3D12_CPU_DESCRIPTOR_HANDLE*  mainRenderTargetDescriptor;

public:
	App();

	void Run();
	bool Init();

	~App();

private:
	void InitWindow();
	bool InitD3D();
	void InitImgui();
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	void WaitForLastSubmittedFrame();
	FrameContext* WaitForNextFrameResources();
	static LRESULT CALLBACK WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};