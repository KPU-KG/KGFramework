#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "hash.h"
#include "ResourceContainer.h"
#include "DescriptorHeapManager.h"
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
		bool useCubeRender = false;
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
		union
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE rtvCubeHandle[6];
		};

		//GBuffer
		std::array<ID3D12Resource*, 4> gbufferTextures = { nullptr, nullptr, nullptr, nullptr };
		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> gbufferHandle;
		ID3D12DescriptorHeap* gbufferDescriptorHeap = nullptr;
		ID3D12DescriptorHeap* gbufferSRVHeap = nullptr;

		//Depth
		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	private:
		void CreateRenderTarget()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			this->renderTarget = 
				this->desc.useCubeRender ? 
				CreateCubeRenderTargetResource(device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM )
				: CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
		}

		void CreateRenderTargetView()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();

			D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
			ZeroDesc( d3dDescriptorHeapDesc );

			d3dDescriptorHeapDesc.NumDescriptors = this->desc.useCubeRender ? 6 : 1;
			d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			d3dDescriptorHeapDesc.NodeMask = 0;

			HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->rtvDescriptorHeap ) );

			if ( this->desc.useCubeRender )
			{
				D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Format = this->renderTarget->GetDesc().Format;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.PlaneSlice = 0;


				for ( size_t i = 0; i < 6; i++ )
				{
					rtvDesc.Texture2DArray.FirstArraySlice = i;
					rtvDesc.Texture2DArray.ArraySize = 1;

					device->CreateRenderTargetView( this->renderTarget, &rtvDesc, rtvCpuDescHandle );
					this->rtvCubeHandle[i] = rtvCpuDescHandle;
					rtvCpuDescHandle.ptr += KGDXRenderer::GetInstance()->GetRTVSize();
				}
			}
			else 
			{
				D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				device->CreateRenderTargetView( this->renderTarget, nullptr, rtvCpuDescHandle );
				this->rtvHandle = rtvCpuDescHandle;
			}
		}

		void CreateGBuffer()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			this->gbufferTextures[0] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
			this->gbufferTextures[1] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
			this->gbufferTextures[2] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R16G16_SNORM );
			this->gbufferTextures[3] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
		}

		void CreateGBufferRTView()
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

		void CreateGBufferSRView()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			auto srvSize = KGDXRenderer::GetInstance()->GetSRVSize();

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroDesc( srvDesc );

			D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
			ZeroDesc( d3dDescriptorHeapDesc );

			d3dDescriptorHeapDesc.NumDescriptors = 5;
			d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			d3dDescriptorHeapDesc.NodeMask = 0;

			HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->gbufferSRVHeap ) );

			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = this->gbufferSRVHeap->GetCPUDescriptorHandleForHeapStart();
			for ( size_t i = 0; i < 4; i++ )
			{
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = this->gbufferTextures[i]->GetDesc().Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = this->gbufferTextures[i]->GetDesc().MipLevels;
				srvDesc.Texture2D.ResourceMinLODClamp = 0;
				device->CreateShaderResourceView( this->gbufferTextures[i], &srvDesc, cpuHandle );
				cpuHandle.ptr += srvSize;
			}

			//±íÀÌ G¹öÆÛ
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = this->depthStencilBuffer->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0;
			device->CreateShaderResourceView( this->depthStencilBuffer, &srvDesc, cpuHandle );

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
			auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = this->renderTarget->GetDesc().Format;
			srvDesc.ViewDimension = this->desc.useCubeRender ? D3D12_SRV_DIMENSION_TEXTURE2D : D3D12_SRV_DIMENSION_TEXTURECUBE;
			if ( this->desc.useCubeRender )
			{
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = 1;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			}
			else
			{
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.PlaneSlice = 0;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			}
			resourceContainer->CreateTexture( this->desc.renderTargetTextureId, this->renderTarget, srvDesc );
		}

		KG::Resource::Texture* PostDepthStencilTexture()
		{
			auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = this->renderTarget->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			
			resourceContainer->CreateTexture( this->desc.depthBufferTextureId, this->depthStencilBuffer, srvDesc );
		}

	public:
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
				this->CreateGBufferRTView();
				this->CreateGBufferSRView();
			}
			if ( this->desc.useDepthStencilBuffer )
			{
				this->CreateDepthStencilBuffer();
				this->CreateDepthStencilBufferView();
			}
			if ( this->desc.renderTargetTextureId.value )
			{
				this->PostRenderTargetTexture();
			}
			if ( this->desc.depthBufferTextureId.value )
			{
				this->PostDepthStencilTexture();
			}
		}

		void CopyGBufferSRV()
		{
			auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
			auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
			device->CopyDescriptorsSimple( 5,
				this->gbufferSRVHeap->GetCPUDescriptorHandleForHeapStart(),
				descManager->GetCPUHandle( 0 ),
				D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
			);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetRTVHandle( size_t index = 0 )
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(index, KGDXRenderer::GetInstance()->GetRTVSize());
			return handle;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferRTVHandle( size_t index )
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE( this->gbufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
			handle.Offset( index, KGDXRenderer::GetInstance()->GetRTVSize() );
			return handle;
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