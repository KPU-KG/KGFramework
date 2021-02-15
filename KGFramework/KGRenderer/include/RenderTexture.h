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
	//리팩토링 필요!
	struct RenderTexture
	{
		RenderTextureDesc desc;

		//RenderTarget
		ID3D12Resource* renderTarget = nullptr;
		ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
		UINT renderTargetSRVIndex = -1;
		KG::Resource::Texture* renderTargetTexture = nullptr;

		//GBuffer
		std::array<ID3D12Resource*, 4> gbufferTextures = { nullptr, nullptr, nullptr, nullptr };
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> gbufferHandle;
		ID3D12DescriptorHeap* gbufferDescriptorHeap = nullptr;
		UINT gbufferSRVIndex = -1;

		//Depth
		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		UINT depthStencilSRVIndex = -1;
		KG::Resource::Texture* depthStencilTexture = nullptr;

		//Barrier
		D3D12_RESOURCE_STATES renderTargetState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES gbufferState = D3D12_RESOURCE_STATE_COMMON;
		D3D12_RESOURCE_STATES depthStencilState = D3D12_RESOURCE_STATE_COMMON;
		std::array<D3D12_RESOURCE_BARRIER, 6> resourceBarrier;

	private:
		void CreateRenderTarget();

		void CreateRenderTargetView();

		void CreateGBuffer();

		void CreateGBufferRTView();

		void CreateGBufferSRView();

		void CreateDepthStencilBuffer();

		void CreateDepthStencilBufferView();

		UINT PostRenderTargetSRV();
		KG::Resource::Texture* PostRenderTargetTexture();

		UINT PostDepthStencilSRV();
		KG::Resource::Texture* PostDepthStencilTexture();

	public:
		void Initialize( const RenderTextureDesc& desc );

		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetRTVHandle( size_t index );

		D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTVHandle( size_t index );

		std::pair<size_t, D3D12_RESOURCE_BARRIER*> BarrierTransition(
			D3D12_RESOURCE_STATES renderTargetState,
			D3D12_RESOURCE_STATES gbufferState,
			D3D12_RESOURCE_STATES depthStencilState
		);

		void ClearGBuffer( ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a );

		bool isCubeDepth();

		void Release();
	};

}