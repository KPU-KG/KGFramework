#pragma once
#include <d3d12.h>
#include <vector>
#include "KGRenderer.h"
namespace KG::Renderer
{
	using Microsoft::WRL::ComPtr;
	using std::vector;
	class KGDXRenderer : public IKGRenderer
	{
	private:
		IDXGIFactory4* dxgiFactory = nullptr;
		IDXGISwapChain3* swapChain = nullptr;

		ID3D12Device* d3dDevice = nullptr;

		UINT swapChainBufferIndex = 0;

		vector<ID3D12Resource*> renderTargetBuffers;
		ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
		UINT rtvDescriptorSize = 0;

		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		UINT dsvDescriptoSize = 0;
		
		ID3D12CommandQueue* commandQueue = nullptr;
		ID3D12CommandAllocator* mainCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* mainCommandList = nullptr;

		ID3D12Fence* fence = nullptr;
		UINT64 fenceValue = 0;
		HANDLE hFenceEvent = 0;

		bool isWireFrame = false;
	private:
		void CreateD3DDevice();
		void CreateSwapChain();
		void CreateRtvDescriptorHeaps();
		void CreateDsvDescriptorHeaps();
		void CreateCommandQueueAndList();
		void CreateRenderTargetView();
		void CreateDepthStencilView();

	public:
		KGDXRenderer();

		virtual void Initialize() override;
		virtual void Render() override;
		virtual void Update() override;
		virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;
	};
}