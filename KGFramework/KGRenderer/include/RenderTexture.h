#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "hash.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "KGDX12Resource.h"
#include "KGRenderer.h"

namespace KG::Resource
{
	struct Texture;
}

namespace KG::Renderer
{
	struct RenderTextureDesc;
	//리팩토링 필요!
	struct RenderTexture
	{
		RenderTextureDesc desc;

		//RenderTarget
		DescriptorHeapManager rtvDescriptorHeap;

        KG::Resource::DXResource renderTargetResource;
		KG::Resource::Texture* renderTargetTexture = nullptr;

		//GBuffer
        DescriptorHeapManager gbufferDescriptorHeap;
        std::array<KG::Resource::DXResource, 4> gbufferTextureResources;

		//Depth
        DescriptorHeapManager dsvDescriptorHeap;
        KG::Resource::DXResource depthStencilBuffer;
		KG::Resource::Texture* depthStencilTexture = nullptr;

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

        void BarrierTransition(D3D12_RESOURCE_STATES rtv, D3D12_RESOURCE_STATES gbuffer, D3D12_RESOURCE_STATES dsv);

		void ClearGBuffer( ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a );

		bool isCubeDepth();

		void Release();
	};

}