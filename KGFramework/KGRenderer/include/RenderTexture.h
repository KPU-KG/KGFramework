#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "hash.h"
#include "KGRenderer.h"


namespace KG::Resource
{
	struct Texture;
}

namespace KG::Renderer
{
	struct RenderTexture
	{
		size_t currentIndex;
		RenderTextureDesc desc;

		//RenderTarget
		ID3D12Resource* renderTarget = nullptr;
		ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

		//GBuffer
		std::array<ID3D12Resource*, 4> gbufferTextures = { nullptr, nullptr, nullptr, nullptr };
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> gbufferHandle;
		ID3D12DescriptorHeap* gbufferDescriptorHeap = nullptr;
		ID3D12DescriptorHeap* gbufferSRVHeap = nullptr;

		//Depth
		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

		//Barrier
		D3D12_RESOURCE_STATES renderTargetState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		D3D12_RESOURCE_STATES gbufferState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		D3D12_RESOURCE_STATES depthStencilState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		std::array<D3D12_RESOURCE_BARRIER, 6> resourceBarrier;

	private:
		void CreateRenderTarget();

		void CreateRenderTargetView();

		void CreateGBuffer();

		void CreateGBufferRTView();

		void CreateGBufferSRView();

		void CreateDepthStencilBuffer();

		void CreateDepthStencilBufferView();

		KG::Resource::Texture* PostRenderTargetTexture();

		KG::Resource::Texture* PostDepthStencilTexture();

	public:
		void Initialize( const RenderTextureDesc& desc );

		void CopyGBufferSRV();

		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetRTVHandle();

		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetRTVHandle( size_t index );

		D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTVHandle( size_t index );

		std::pair<size_t, D3D12_RESOURCE_BARRIER*> BarrierTransition(
			D3D12_RESOURCE_STATES renderTargetState,
			D3D12_RESOURCE_STATES gbufferState,
			D3D12_RESOURCE_STATES depthStencilState
		);

		void ClearGBuffer( ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a );

		void Release();
	};

}