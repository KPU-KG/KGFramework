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
    //auto format = this->desc.useHDR ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
    auto format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    if ( this->desc.useCubeRender || this->desc.useGSArrayRender )
	{
		this->renderTargetResource.resource = CreateArrayRenderTargetResource(device, this->desc.width, this->desc.height, this->desc.length, format);
        this->renderTargetResource.currentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
    }
	else
	{
		this->renderTargetResource.resource = CreateRenderTargetResource(device, this->desc.width, this->desc.height, format);
        this->renderTargetResource.currentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
    }
}

void KG::Renderer::RenderTexture::CreateRenderTargetView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc(d3dDescriptorHeapDesc);

	d3dDescriptorHeapDesc.NumDescriptors = this->desc.length;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

    rtvDescriptorHeap.Initialize(device, d3dDescriptorHeapDesc, KGDXRenderer::GetInstance()->GetHWFeature().rtvDescriptorSize);
	if ( this->desc.useCubeRender )
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Format = this->renderTargetResource->GetDesc().Format;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;

		for ( size_t i = 0; i < 6; i++ )
		{
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			rtvDesc.Texture2DArray.ArraySize = 1;
            this->renderTargetResource.AddOnDescriptorHeap(&rtvDescriptorHeap, rtvDesc);
		}
	}
	else
	{
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Format = this->renderTargetResource->GetDesc().Format;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        this->renderTargetResource.AddOnDescriptorHeap(&rtvDescriptorHeap, rtvDesc);
	}
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
}

void KG::Renderer::RenderTexture::CreateGBuffer()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	this->gbufferTextureResources[0].resource = CreateRenderTargetResource(device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM);
    this->gbufferTextureResources[0].currentState  = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

	this->gbufferTextureResources[1].resource = CreateRenderTargetResource(device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UNORM);
    this->gbufferTextureResources[1].currentState  = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

	this->gbufferTextureResources[2].resource = CreateRenderTargetResource(device, this->desc.width, this->desc.height, DXGI_FORMAT_R16G16_SNORM);
    this->gbufferTextureResources[2].currentState  = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

	this->gbufferTextureResources[3].resource = CreateRenderTargetResource(device, this->desc.width, this->desc.height, DXGI_FORMAT_R8G8B8A8_UINT);
    this->gbufferTextureResources[3].currentState  = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
}

void KG::Renderer::RenderTexture::CreateGBufferRTView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto rtvSize = KGDXRenderer::GetInstance()->GetHWFeature().rtvDescriptorSize;

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc(d3dDescriptorHeapDesc);

	d3dDescriptorHeapDesc.NumDescriptors = 4;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

    gbufferDescriptorHeap.Initialize(device, d3dDescriptorHeapDesc, rtvSize);

	for ( size_t i = 0; i < 4; i++ )
	{
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Format = gbufferTextureResources[i]->GetDesc().Format;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;
        this->gbufferTextureResources[i].AddOnDescriptorHeap(&gbufferDescriptorHeap, rtvDesc);
	}
}

void KG::Renderer::RenderTexture::CreateGBufferSRView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetHWFeature().srvDescriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc(srvDesc);

	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
	for ( size_t i = 0; i < 4; i++ )
	{
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = this->gbufferTextureResources[i]->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = this->gbufferTextureResources[i]->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0;
        this->gbufferTextureResources[i].AddOnDescriptorHeap(descManager, srvDesc);
	}

	//깊이 G버퍼
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = this->depthStencilBuffer->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;
    this->depthStencilBuffer.AddOnDescriptorHeap(descManager, srvDesc);
	//device->CreateShaderResourceView(this->depthStencilBuffer, &srvDesc, descManager->GetCPUHandle(this->gbufferSRVIndex + 4));

}

void KG::Renderer::RenderTexture::CreateDepthStencilBuffer()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();

	if ( this->desc.useCubeRender || this->desc.useGSArrayRender )
	{
		this->depthStencilBuffer.resource = CreateArrayDepthStencilResource(device, this->desc.width, this->desc.height, this->desc.length);
        this->depthStencilBuffer.currentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
    }
	else
	{
		this->depthStencilBuffer.resource = CreateDepthStencilResource(device, this->desc.width, this->desc.height);
        this->depthStencilBuffer.currentState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
    }
}

void KG::Renderer::RenderTexture::CreateDepthStencilBufferView()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto dsvSize = KGDXRenderer::GetInstance()->GetHWFeature().dsvDescriptorSize;

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc(d3dDescriptorHeapDesc);

	d3dDescriptorHeapDesc.NumDescriptors = this->isCubeDepth() && !(this->desc.useGSCubeRender || this->desc.useGSArrayRender) ? this->desc.length : 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

    dsvDescriptorHeap.Initialize(device, d3dDescriptorHeapDesc, dsvSize);

	if ( this->isCubeDepth() || this->desc.useGSArrayRender )
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencillViewDesc;
		d3dDepthStencillViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		d3dDepthStencillViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		d3dDepthStencillViewDesc.Texture2DArray.MipSlice = 0;
		d3dDepthStencillViewDesc.Flags = D3D12_DSV_FLAG_NONE;

		if ( this->desc.useGSCubeRender || this->desc.useGSArrayRender )
		{
			d3dDepthStencillViewDesc.Texture2DArray.ArraySize = this->desc.length;
			d3dDepthStencillViewDesc.Texture2DArray.FirstArraySlice = 0;
            depthStencilBuffer.AddOnDescriptorHeap(&this->dsvDescriptorHeap, d3dDepthStencillViewDesc);
		}
		else
		{
			for ( size_t i = 0; i < this->desc.length; i++ )
			{
				d3dDepthStencillViewDesc.Texture2DArray.FirstArraySlice = i;
				d3dDepthStencillViewDesc.Texture2DArray.ArraySize = 1;

                depthStencilBuffer.AddOnDescriptorHeap(&this->dsvDescriptorHeap, d3dDepthStencillViewDesc);
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
        depthStencilBuffer.AddOnDescriptorHeap(&this->dsvDescriptorHeap, d3dDepthStencillViewDesc);
	}
}

UINT KG::Renderer::RenderTexture::PostRenderTargetSRV()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetHWFeature().srvDescriptorSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc(srvDesc);

	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = this->renderTargetResource->GetDesc().Format;

	srvDesc.ViewDimension = this->desc.useCubeRender ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = this->renderTargetResource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;

    this->renderTargetResource.AddOnDescriptorHeap(descManager, srvDesc);
	auto ret =  this->renderTargetResource.GetDescriptor(KG::Resource::DescriptorType::SRV).HeapIndex;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    ZeroDesc(uavDesc);
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = this->renderTargetResource->GetDesc().Format;
    this->renderTargetResource.AddOnDescriptorHeap(descManager, uavDesc);

    return ret;
}

KG::Resource::Texture* KG::Renderer::RenderTexture::PostRenderTargetTexture()
{
	auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = this->renderTargetResource->GetDesc().Format;
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
	this->renderTargetTexture = resourceContainer->CreateTexture(this->desc.renderTargetTextureId, this->renderTargetResource.resource, srvDesc);
	return this->renderTargetTexture;
}

UINT KG::Renderer::RenderTexture::PostDepthStencilSRV()
{
	auto device = KGDXRenderer::GetInstance()->GetD3DDevice();
	auto srvSize = KGDXRenderer::GetInstance()->GetHWFeature().srvDescriptorSize;
	auto descManager = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroDesc(srvDesc);

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	if ( this->isCubeDepth() )
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	}
	else if ( this->desc.useGSArrayRender )
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	if ( this->desc.useCubeRender )
	{
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else if ( this->desc.useGSArrayRender )
	{
		srvDesc.Texture2DArray.ArraySize = this->desc.length;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
	}
	else
	{
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
    this->depthStencilBuffer.AddOnDescriptorHeap(descManager, srvDesc);
    if (this->desc.useGSArrayRender)
    {
        for (size_t i = 0; i < this->desc.length; i++)
        {
            srvDesc.Texture2DArray.ArraySize = 1;
            srvDesc.Texture2DArray.FirstArraySlice = i;
            srvDesc.Texture2DArray.MipLevels = 1;
            this->depthStencilBuffer.AddOnDescriptorHeap(descManager, srvDesc);
        }
    }
	return this->depthStencilBuffer.GetDescriptor(KG::Resource::DescriptorType::SRV).HeapIndex;
}

KG::Resource::Texture* KG::Renderer::RenderTexture::PostDepthStencilTexture()
{
	auto resourceContainer = KG::Resource::ResourceContainer::GetInstance();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	if ( this->isCubeDepth() )
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	}
	else if ( this->desc.useGSArrayRender )
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	}
	if ( this->desc.useCubeRender )
	{
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else if ( this->desc.useGSArrayRender )
	{
		srvDesc.Texture2DArray.ArraySize = this->desc.length;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
	}
	else
	{
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}


	this->depthStencilTexture = resourceContainer->CreateTexture(this->desc.depthBufferTextureId, this->depthStencilBuffer, srvDesc);
	return this->depthStencilTexture;
}

void KG::Renderer::RenderTexture::Initialize(const RenderTextureDesc& desc)
{
	this->desc = desc;
    if ( this->desc.useScreenSize )
    {
        this->desc.width = KG::Renderer::KGDXRenderer::GetInstance()->GetSetting().clientWidth;
        this->desc.height = KG::Renderer::KGDXRenderer::GetInstance()->GetSetting().clientHeight;
    }
	if ( this->desc.useCubeRender )
	{
		this->desc.length = 6;
	}
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
	else if ( this->desc.uploadSRVRenderTarget || this->desc.useRenderTarget)
	{
		this->PostRenderTargetSRV();
	}

	if ( this->desc.depthBufferTextureId.value )
	{
		this->PostDepthStencilTexture();
	}
	else if ( this->desc.uploadSRVDepthBuffer || this->desc.useDepthStencilBuffer)
	{
		this->PostDepthStencilSRV();
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::RenderTexture::GetRenderTargetRTVHandle(size_t index)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(index, KGDXRenderer::GetInstance()->GetHWFeature().rtvDescriptorSize);
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::RenderTexture::GetGBufferRTVHandle(size_t index)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->gbufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	handle.Offset(index, KGDXRenderer::GetInstance()->GetHWFeature().rtvDescriptorSize);
	return handle;
}

void KG::Renderer::RenderTexture::BarrierTransition(D3D12_RESOURCE_STATES rtv, D3D12_RESOURCE_STATES gbuffer, D3D12_RESOURCE_STATES dsv)
{
    if(this->renderTargetResource.resource) this->renderTargetResource.AddTransitionQueue(rtv);
    if(this->gbufferTextureResources[0].resource) this->gbufferTextureResources[0].AddTransitionQueue(gbuffer);
    if(this->gbufferTextureResources[1].resource) this->gbufferTextureResources[1].AddTransitionQueue(gbuffer);
    if(this->gbufferTextureResources[2].resource) this->gbufferTextureResources[2].AddTransitionQueue(gbuffer);
    if(this->gbufferTextureResources[3].resource) this->gbufferTextureResources[3].AddTransitionQueue(gbuffer);
    if(this->depthStencilBuffer.resource) this->depthStencilBuffer.AddTransitionQueue(dsv);
}

void KG::Renderer::RenderTexture::ClearGBuffer(ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a)
{
	float clearGbufferColor[4] = { r, g, b, a };
	for ( size_t i = 0; i < 4; i++ )
	{
		cmdList->ClearRenderTargetView(this->GetGBufferRTVHandle(i), clearGbufferColor, 0, nullptr);
	}
}

bool KG::Renderer::RenderTexture::isCubeDepth()
{
	return this->desc.useCubeRender && (this->desc.uploadSRVDepthBuffer || this->desc.depthBufferTextureId.value != 0);
}

void KG::Renderer::RenderTexture::Release()
{
    renderTargetResource.Release();
    for (auto& i : gbufferTextureResources)
    {
        i.Release();
    }
    depthStencilBuffer.Release();
}

KG::Renderer::RenderTextureProperty::RenderTextureProperty(KG::Renderer::RenderTextureDesc& ref)
	:
	CONST_KG_PROPERTY(textureWidth, ref.width),
	CONST_KG_PROPERTY(textureHeight, ref.height),
	CONST_KG_PROPERTY(arrayCount, ref.length),
    CONST_KG_PROPERTY(useScreenSize, ref.useScreenSize),
    CONST_KG_PROPERTY(useCubeRender, ref.useCubeRender),
    CONST_KG_PROPERTY(useGSCubeRender, ref.useGSCubeRender),
	CONST_KG_PROPERTY(useGSArrayRender, ref.useGSArrayRender),
	CONST_KG_PROPERTY(useRenderTarget, ref.useRenderTarget),
	CONST_KG_PROPERTY(useDeferredRender, ref.useDeferredRender),
    CONST_KG_PROPERTY(useHDR, ref.useHDR),
	CONST_KG_PROPERTY(useDepthStencilBuffer, ref.useDepthStencilBuffer),
	CONST_KG_PROPERTY(uploadSRVRenderTarget, ref.uploadSRVRenderTarget),
	CONST_KG_PROPERTY(uploadSRVDepthBuffer, ref.uploadSRVDepthBuffer),
	CONST_KG_PROPERTY(renderTargetTextureId, ref.renderTargetTextureId),
    CONST_KG_PROPERTY(depthBufferTextureId, ref.depthBufferTextureId)
{
}

void KG::Renderer::RenderTextureProperty::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	auto* renderTextureDesc = objectElement->FirstChildElement(this->name.c_str());
	textureWidth.OnDataLoad(renderTextureDesc);
	textureHeight.OnDataLoad(renderTextureDesc);
	arrayCount.OnDataLoad(renderTextureDesc);
    useScreenSize.OnDataLoad(renderTextureDesc);
    useCubeRender.OnDataLoad(renderTextureDesc);
    useGSCubeRender.OnDataLoad(renderTextureDesc);
	useGSArrayRender.OnDataLoad(renderTextureDesc);
	useRenderTarget.OnDataLoad(renderTextureDesc);
	useDeferredRender.OnDataLoad(renderTextureDesc);
    useDepthStencilBuffer.OnDataLoad(renderTextureDesc);
    useHDR.OnDataLoad(renderTextureDesc);
    uploadSRVRenderTarget.OnDataLoad(renderTextureDesc);
	uploadSRVDepthBuffer.OnDataLoad(renderTextureDesc);
	renderTargetTextureId.OnDataLoad(renderTextureDesc);
	depthBufferTextureId.OnDataLoad(renderTextureDesc);
}

void KG::Renderer::RenderTextureProperty::SetName(const std::string& newName)
{
	this->name = name;
}

void KG::Renderer::RenderTextureProperty::OnDataSave(tinyxml2::XMLElement* objectElement)
{
	auto* renderTextureDesc = objectElement->InsertNewChildElement(this->name.c_str());
	textureWidth.OnDataSave(renderTextureDesc);
	textureHeight.OnDataSave(renderTextureDesc);
	arrayCount.OnDataSave(renderTextureDesc);
    useScreenSize.OnDataSave(renderTextureDesc);
    useCubeRender.OnDataSave(renderTextureDesc);
	useGSCubeRender.OnDataSave(renderTextureDesc);
	useGSArrayRender.OnDataSave(renderTextureDesc);
	useRenderTarget.OnDataSave(renderTextureDesc);
	useDeferredRender.OnDataSave(renderTextureDesc);
	useDepthStencilBuffer.OnDataSave(renderTextureDesc);
    useHDR.OnDataSave(renderTextureDesc);
	uploadSRVRenderTarget.OnDataSave(renderTextureDesc);
	uploadSRVDepthBuffer.OnDataSave(renderTextureDesc);
	renderTargetTextureId.OnDataSave(renderTextureDesc);
	depthBufferTextureId.OnDataSave(renderTextureDesc);
}

bool KG::Renderer::RenderTextureProperty::OnDrawGUI()
{
	bool flag = false;
	if ( ImGui::TreeNode("RenderTextureDesc") )
	{
		//렌더텍스처 문단으로 들어갈 시간
		flag |= textureWidth.OnDrawGUI();
		flag |= textureHeight.OnDrawGUI();
		flag |= arrayCount.OnDrawGUI();
        flag |= useScreenSize.OnDrawGUI();
        flag |= useCubeRender.OnDrawGUI();
        flag |= useGSCubeRender.OnDrawGUI();
		flag |= useGSArrayRender.OnDrawGUI();
		flag |= useRenderTarget.OnDrawGUI();
		flag |= useDeferredRender.OnDrawGUI();
        flag |= useDepthStencilBuffer.OnDrawGUI();
        flag |= useHDR.OnDrawGUI();
        flag |= uploadSRVRenderTarget.OnDrawGUI();
		flag |= uploadSRVDepthBuffer.OnDrawGUI();
		flag |= renderTargetTextureId.OnDrawGUI();
		flag |= depthBufferTextureId.OnDrawGUI();
		ImGui::TreePop();
	}
	return flag;
}
