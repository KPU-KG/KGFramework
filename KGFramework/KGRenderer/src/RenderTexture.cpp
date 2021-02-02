#include "pch.h"
#include "KGDXRenderer.h"
#include "ResourceContainer.h"
#include "DescriptorHeapManager.h"
#include "ResourceMetaData.h"
#include "RenderTexture.h"

using KG::Renderer::KGDXRenderer;

void KG::Renderer::RenderTexture::CreateRenderTarget()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	this->renderTarget =
		this->desc.useCubeRender ?
		CreateCubeRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM )
		: CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
}

void KG::Renderer::RenderTexture::CreateRenderTargetView()
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
			rtvCpuDescHandle.ptr += KGDXRenderer::GetInstance()->GetRTVSize();
		}
	}
	else
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		device->CreateRenderTargetView( this->renderTarget, nullptr, rtvCpuDescHandle );
	}
}

void KG::Renderer::RenderTexture::CreateGBuffer()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	this->gbufferTextures[0] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
	this->gbufferTextures[1] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM );
	this->gbufferTextures[2] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R16G16_SNORM );
	this->gbufferTextures[3] = CreateRenderTargetResource( device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UINT );
}

void KG::Renderer::RenderTexture::CreateGBufferRTView()
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

void KG::Renderer::RenderTexture::CreateGBufferSRView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetSRVSize();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc( srvDesc );

	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
	this->gbufferSRVIndex = descManager->RequestEmptyIndex();
	descManager->RequestEmptyIndex();
	descManager->RequestEmptyIndex();
	descManager->RequestEmptyIndex();
	descManager->RequestEmptyIndex();

	for ( size_t i = 0; i < 4; i++ )
	{
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = this->gbufferTextures[i]->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = this->gbufferTextures[i]->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0;
		device->CreateShaderResourceView( this->gbufferTextures[i], &srvDesc, descManager->GetCPUHandle( this->gbufferSRVIndex + i ) );
	}

	//±íÀÌ G¹öÆÛ
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = this->depthStencilBuffer->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;
	device->CreateShaderResourceView( this->depthStencilBuffer, &srvDesc, descManager->GetCPUHandle( this->gbufferSRVIndex + 4 ) );

}

void KG::Renderer::RenderTexture::CreateDepthStencilBuffer()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	this->depthStencilBuffer = this->isCubeDepth() ?
		CreateCubeDepthStencilResource( device, this->desc.width, this->desc.height )
		: CreateDepthStencilResource( device, this->desc.width, this->desc.height );
}

void KG::Renderer::RenderTexture::CreateDepthStencilBufferView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto rtvSize = KGDXRenderer::GetInstance()->GetRTVSize();

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc( d3dDescriptorHeapDesc );

	d3dDescriptorHeapDesc.NumDescriptors = this->isCubeDepth() && !this->desc.useGSCubeRender ? 6 : 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = device->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->dsvDescriptorHeap ) );


	if ( this->isCubeDepth() )
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencillViewDesc;
		d3dDepthStencillViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dDepthStencillViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		d3dDepthStencillViewDesc.Texture2DArray.MipSlice = 0;
		d3dDepthStencillViewDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDescHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		this->dsvHandle = dsvCpuDescHandle;
		if ( this->desc.useGSCubeRender )
		{
			d3dDepthStencillViewDesc.Texture2DArray.ArraySize = 6;
			d3dDepthStencillViewDesc.Texture2DArray.FirstArraySlice = 0;
			device->CreateDepthStencilView( this->depthStencilBuffer, &d3dDepthStencillViewDesc, dsvCpuDescHandle );
		}
		else 
		{
			for ( size_t i = 0; i < 6; i++ )
			{
				d3dDepthStencillViewDesc.Texture2DArray.FirstArraySlice = i;
				d3dDepthStencillViewDesc.Texture2DArray.ArraySize = 1;

				device->CreateDepthStencilView( this->depthStencilBuffer, &d3dDepthStencillViewDesc, dsvCpuDescHandle );
				dsvCpuDescHandle.ptr += KGDXRenderer::GetInstance()->GetDSVSize();
			}
		}
	}
	else
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencillViewDesc;
		d3dDepthStencillViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dDepthStencillViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		d3dDepthStencillViewDesc.Texture2D.MipSlice = 0;
		d3dDepthStencillViewDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuDescHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		device->CreateDepthStencilView( this->depthStencilBuffer, &d3dDepthStencillViewDesc, dsvCpuDescHandle );
		this->dsvHandle = dsvCpuDescHandle;
	}
}

UINT KG::Renderer::RenderTexture::PostRenderTargetSRV()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetSRVSize();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc( srvDesc );

	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = this->renderTarget->GetDesc().Format;
	srvDesc.ViewDimension = this->desc.useCubeRender ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = this->renderTarget->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

	this->renderTargetSRVIndex = descManager->RequestEmptyIndex();
	device->CreateShaderResourceView( this->renderTarget, &srvDesc, descManager->GetCPUHandle( this->renderTargetSRVIndex ) );
	return this->renderTargetSRVIndex;
}

KG::Resource::Texture* KG::Renderer::RenderTexture::PostRenderTargetTexture()
{
	auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = this->renderTarget->GetDesc().Format;
	srvDesc.ViewDimension = this->desc.useCubeRender ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
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
	this->renderTargetTexture = resourceContainer->CreateTexture( this->desc.renderTargetTextureId, this->renderTarget, srvDesc );
	return this->renderTargetTexture;
}

UINT KG::Renderer::RenderTexture::PostDepthStencilSRV()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetSRVSize();
	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc( srvDesc );

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = this->isCubeDepth() ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
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

	this->depthStencilSRVIndex = descManager->RequestEmptyIndex();
	device->CreateShaderResourceView( this->depthStencilBuffer, &srvDesc, descManager->GetCPUHandle( this->depthStencilSRVIndex ) );
	return this->depthStencilSRVIndex;
}

KG::Resource::Texture* KG::Renderer::RenderTexture::PostDepthStencilTexture()
{
	auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = this->isCubeDepth() ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
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

	this->depthStencilTexture = resourceContainer->CreateTexture( this->desc.depthBufferTextureId, this->depthStencilBuffer, srvDesc );
	return this->depthStencilTexture;
}

void KG::Renderer::RenderTexture::Initialize( const RenderTextureDesc& desc )
{
	this->desc = desc;
	if ( this->desc.useRenderTarget )
	{
		this->CreateRenderTarget();
		this->CreateRenderTargetView();
	}
	if ( this->desc.useDepthStencilBuffer )
	{
		this->CreateDepthStencilBuffer();
		this->CreateDepthStencilBufferView();
	}
	if ( this->desc.useDeferredRender )
	{
		this->CreateGBuffer();
		this->CreateGBufferRTView();
		this->CreateGBufferSRView();
	}

	if ( this->desc.renderTargetTextureId.value )
	{
		this->PostRenderTargetTexture();
	}
	else if ( this->desc.uploadSRVRenderTarget )
	{
		this->PostRenderTargetSRV();
	}

	if ( this->desc.depthBufferTextureId.value )
	{
		this->PostDepthStencilTexture();
	}
	else if ( this->desc.uploadSRVDepthBuffer )
	{
		this->PostDepthStencilSRV();
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::RenderTexture::GetRenderTargetRTVHandle( size_t index )
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE( this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
	handle.Offset( index, KGDXRenderer::GetInstance()->GetRTVSize() );
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::RenderTexture::GetGBufferRTVHandle( size_t index )
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE( this->gbufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart() );
	handle.Offset( index, KGDXRenderer::GetInstance()->GetRTVSize() );
	return handle;
}

std::pair<size_t, D3D12_RESOURCE_BARRIER*> KG::Renderer::RenderTexture::BarrierTransition(
	D3D12_RESOURCE_STATES renderTargetState,
	D3D12_RESOURCE_STATES gbufferState,
	D3D12_RESOURCE_STATES depthStencilState
)
{
	size_t csr = 0;
	if ( this->desc.useRenderTarget && renderTargetState != this->renderTargetState )
	{
		this->resourceBarrier[csr] = CD3DX12_RESOURCE_BARRIER::Transition( this->renderTarget, this->renderTargetState, renderTargetState );
		this->renderTargetState = renderTargetState;
		csr += 1;
	}
	if ( this->desc.useDeferredRender && gbufferState != this->gbufferState )
	{
		for ( size_t i = 0; i < 4; i++ )
		{
			this->resourceBarrier[csr] = CD3DX12_RESOURCE_BARRIER::Transition( this->gbufferTextures[i], this->gbufferState, gbufferState );
			csr += 1;
		}
		this->gbufferState = gbufferState;
	}
	if ( this->desc.useDepthStencilBuffer && depthStencilState != this->depthStencilState )
	{
		this->resourceBarrier[csr] = CD3DX12_RESOURCE_BARRIER::Transition( this->depthStencilBuffer, this->depthStencilState, depthStencilState );
		this->depthStencilState = depthStencilState;
		csr += 1;
	}
	return std::pair<size_t, D3D12_RESOURCE_BARRIER*>( csr, this->resourceBarrier.data() );
}

void KG::Renderer::RenderTexture::ClearGBuffer( ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a )
{
	float clearGbufferColor[4] = { r, g, b, a };
	for ( size_t i = 0; i < 4; i++ )
	{
		cmdList->ClearRenderTargetView( this->GetGBufferRTVHandle( i ), clearGbufferColor, 0, nullptr );
	}
}

bool KG::Renderer::RenderTexture::isCubeDepth()
{
	return this->desc.useCubeRender && (this->desc.uploadSRVDepthBuffer || this->desc.depthBufferTextureId.value != 0);
}

void KG::Renderer::RenderTexture::Release()
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
