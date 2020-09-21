#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "hash.h"
#include "ResourceMetaData.h"

namespace KG::Resource
{
	struct Texture;
}

namespace KG::Renderer
{

	struct RenderTextureDesc
	{
		size_t width;
		size_t height;
		size_t count;
		bool useRenderTarget = true;
		bool useDeferredRender = true;
		bool useDepthStencilBuffer = true;
		KG::Utill::HashString renderTargetTextureId = KG::Utill::HashString( 0 );
		KG::Utill::HashString depthBufferTextureId = KG::Utill::HashString( 0 );
	};

	struct RenderTexture
	{
		size_t currentIndex;
		RenderTextureDesc desc;

		//RenderTarget
		ID3D12Resource* renderTarget = nullptr;
		ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;

		//GBuffer
		std::array<ID3D12Resource*, 4> gbufferTextures = { nullptr, nullptr, nullptr, nullptr };
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> gbufferHandle;
		ID3D12DescriptorHeap* gbufferDescriptorHeap = nullptr;

		//Depth
		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

		
		void CreateRenderTarget()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			this->renderTarget = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
		}

		void CreateRenderTargetView()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();

			D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
			ZeroDesc( d3dDescriptorHeapDesc );

			d3dDescriptorHeapDesc.NumDescriptors = 1;
			d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			d3dDescriptorHeapDesc.NodeMask = 0;

			HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->rtvDescriptorHeap ) );

			D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			device->CreateRenderTargetView( this->renderTarget, nullptr, rtvCpuDescHandle );
			this->rtvHandle = rtvCpuDescHandle;
		}

		void CreateGBuffer()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			this->gbufferTextures[0] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
			this->gbufferTextures[1] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
			this->gbufferTextures[2] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R16G16_SNORM );
			this->gbufferTextures[3] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
		}

		void CreateGBufferView()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			auto rtvSize = KGDXRenderer::GetInstance()->GetRTVSize();

			D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
			ZeroDesc( d3dDescriptorHeapDesc );

			d3dDescriptorHeapDesc.NumDescriptors = 4;
			d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			d3dDescriptorHeapDesc.NodeMask = 0;

			HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->gbufferDescriptorHeap ) );

			D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->gbufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			for ( size_t i = 0; i < 4; i++ )
			{
				device->CreateRenderTargetView( this->gbufferTextures[i], nullptr, rtvCpuDescHandle );
				this->gbufferHandle[i] = rtvCpuDescHandle;
				rtvCpuDescHandle.ptr += rtvSize;
			}
		}

		void CreateDepthStencilBuffer()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			this->depthStencilBuffer = CreateDepthStencilResource( device, this->desc.width, this->desc.height);
		}

		void CreateDepthStencilBufferView()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			auto rtvSize = KGDXRenderer::GetInstance()->GetRTVSize();

			D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
			ZeroDesc( d3dDescriptorHeapDesc );

			d3dDescriptorHeapDesc.NumDescriptors = 1;
			d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			d3dDescriptorHeapDesc.NodeMask = 0;

			HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->dsvDescriptorHeap ) );

			D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDescHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			device->CreateRenderTargetView( this->depthStencilBuffer, nullptr, dsvCpuDescHandle );
			this->dsvHandle = dsvCpuDescHandle;
		}

		KG::Resource::Texture* PostRenderTargetTexture()
		{
			//큐브맵 동적 생성시 필요한 절차나 타입들 알아내기
		}

		KG::Resource::Texture* PostDepthStencilTexture()
		{
		}

		void Initialize()
		{
			if ( this->desc.useRenderTarget )
			{
				this->CreateRenderTarget();
				this->CreateRenderTargetView();
			}
			if ( this->desc.useDeferredRender )
			{
				this->CreateGBuffer();
				this->CreateGBufferView();
			}
			if ( this->desc.useDepthStencilBuffer )
			{
				this->CreateDepthStencilBuffer();
				this->CreateDepthStencilBufferView();
			}
		}

		void Release()
		{
			TryRelease( this->renderTarget );
			TryRelease( this->rtvDescriptorHeap );

			TryRelease( this->depthStencilBuffer );
			TryRelease( this->dsvDescriptorHeap );

			for ( size_t i = 0; i < 4; i++ )
			{
				TryRelease( this->gbufferTextures[i] );
			}
			TryRelease( this->gbufferDescriptorHeap );
		}
	};

}