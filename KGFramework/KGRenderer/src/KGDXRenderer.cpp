#include "pch.h"
#include "KGDXRenderer.h"
#include "Debug.h"
#include "D3D12Helper.h"
#include <string>
#include <sstream>
#include "KGRenderQueue.h"
#include "GraphicSystem.h"
#include "ResourceContainer.h"


using namespace KG::Renderer;

struct KG::Renderer::KGDXRenderer::GraphicSystems
{
	KG::System::Render3DSystem render3DSystem;
	KG::System::GeometrySystem geometrySystem;
	KG::System::MaterialSystem materialSystem;
};


KG::Renderer::KGDXRenderer::KGDXRenderer()
{
	KG::Resource::ResourceContainer::GetInstance();
}

KG::Renderer::KGDXRenderer::~KGDXRenderer()
{
	KG::Resource::ResourceContainer::GetInstance()->Clear();
	TryRelease( dxgiFactory );
	TryRelease( swapChain );
	TryRelease( d3dDevice );
	TryRelease( rtvDescriptorHeap );
	TryRelease( depthStencilBuffer );
	TryRelease( dsvDescriptorHeap );
	TryRelease( commandQueue );
	TryRelease( mainCommandAllocator );
	TryRelease( mainCommandList );
	TryRelease( generalRootSignature );
	TryRelease( fence );

	for ( auto*& ptr : this->renderTargetBuffers )
	{
		TryRelease( ptr );
	}
	DebugNormalMessage( "Close D3D12 Renderer" );
}

void KGDXRenderer::Initialize()
{
	KGDXRenderer::instance = this;
	DebugNormalMessage( "Initilize D3D12 Renderer" );
	this->CreateD3DDevice();
	this->QueryHardwareFeature();
	this->CreateCommandQueueAndList();
	this->CreateSwapChain();
	this->CreateRtvDescriptorHeaps();
	this->CreateDsvDescriptorHeaps();
	this->CreateRenderTargetView();
	this->CreateDepthStencilView();
	this->CreateGeneralRootSignature();
	this->renderEngine = std::make_unique<KGRenderEngine>( this->d3dDevice );
	this->graphicSystems = std::make_unique<GraphicSystems>();
}

void KGDXRenderer::Render()
{
	HRESULT hResult = this->mainCommandAllocator->Reset();
	hResult = this->mainCommandList->Reset( this->mainCommandAllocator, nullptr );

	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory( &d3dResourceBarrier, sizeof( decltype(d3dResourceBarrier) ) );
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE::D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = this->renderTargetBuffers[this->swapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	this->mainCommandList->ResourceBarrier( 1, &d3dResourceBarrier );

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (this->swapChainBufferIndex * this->rtvDescriptorSize);

	float clearColor[4] = { 0.0f, 0.0f, 25.0f / 256.0f, 1.0f };
	this->mainCommandList->ClearRenderTargetView( d3dRtvCPUDescriptorHandle, clearColor, 0, nullptr );

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	this->mainCommandList->ClearDepthStencilView( d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );

	this->mainCommandList->OMSetRenderTargets( 1, &d3dRtvCPUDescriptorHandle, true, &d3dDsvCPUDescriptorHandle );

	this->mainCommandList->SetGraphicsRootSignature( this->generalRootSignature );

	this->renderEngine->Render( this->mainCommandList );

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	this->mainCommandList->ResourceBarrier( 1, &d3dResourceBarrier );

	hResult = this->mainCommandList->Close();

	ID3D12CommandList* d3dCommandLists[] = { this->mainCommandList };
	this->commandQueue->ExecuteCommandLists( 1, d3dCommandLists );


	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = nullptr;
	dxgiPresentParameters.pScrollRect = nullptr;
	dxgiPresentParameters.pScrollOffset = nullptr;
	this->swapChain->Present1( this->setting.isVsync, 0, &dxgiPresentParameters );

	this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();

	this->MoveToNextFrame();
}

void KG::Renderer::KGDXRenderer::Update()
{
	static bool impleFlag = false;
	if ( !impleFlag )
	{
		NotImplement( KGDXRenderer::Update );
		impleFlag = !impleFlag;
	}
}

void KGDXRenderer::OnChangeSettings( const RendererSetting& prev, const RendererSetting& next )
{
	NotImplement( KGDXRenderer::OnChangeSettings );
}

KG::Component::Render3DComponent* KG::Renderer::KGDXRenderer::GetNewRenderComponent()
{
	return static_cast<KG::Component::Render3DComponent*>(this->graphicSystems->render3DSystem.GetNewComponent());
}

KG::Component::GeometryComponent* KG::Renderer::KGDXRenderer::GetNewGeomteryComponent( const KG::Utill::HashString& id )
{
	auto* geo = static_cast<KG::Component::GeometryComponent*>(this->graphicSystems->geometrySystem.GetNewComponent());
	geo->InitializeGeometry( id );
	return geo;
}

KG::Component::MaterialComponent* KG::Renderer::KGDXRenderer::GetNewMaterialComponent( const KG::Utill::HashString& id )
{
	auto* mat = static_cast<KG::Component::MaterialComponent*>(this->graphicSystems->materialSystem.GetNewComponent());
	mat->InitializeShader( id );
	return mat;
}

KG::Component::CameraComponent* KG::Renderer::KGDXRenderer::GetNewCameraComponent()
{
	return nullptr;
}

KG::Component::LightComponent* KG::Renderer::KGDXRenderer::GetNewLightComponent()
{
	return nullptr;
}

KG::Renderer::KGDXRenderer* KG::Renderer::KGDXRenderer::GetInstance()
{
	return KGDXRenderer::instance;
}

ID3D12RootSignature* KG::Renderer::KGDXRenderer::GetGeneralRootSignature() const
{
	return this->generalRootSignature;
}

KGRenderEngine* KG::Renderer::KGDXRenderer::GetRenderEngine() const
{
	return this->renderEngine.get();
}

void KG::Renderer::KGDXRenderer::QueryHardwareFeature()
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS featureSupport;
	ZeroDesc( featureSupport );

	d3dDevice->CheckFeatureSupport( D3D12_FEATURE_D3D12_OPTIONS, &featureSupport, sizeof( featureSupport ) );

	switch ( featureSupport.ResourceBindingTier )
	{
	case D3D12_RESOURCE_BINDING_TIER_1:
	{
		DebugNormalMessage( "D3D12 RESOURCE BINDING TIER : 1" );
	}
	break;

	case D3D12_RESOURCE_BINDING_TIER_2:
	{
		DebugNormalMessage( "D3D12 RESOURCE BINDING TIER : 2" );
	}
	break;

	case D3D12_RESOURCE_BINDING_TIER_3:
	{
		DebugNormalMessage( "D3D12 RESOURCE BINDING TIER: 3" );
	}
	break;
	}

}

void KG::Renderer::KGDXRenderer::CreateD3DDevice()
{
	HRESULT hResult;
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) | defined(DEBUF)
	ID3D12Debug* d3dDebugController = nullptr;
	hResult = D3D12GetDebugInterface( IID_PPV_ARGS( &d3dDebugController ) );
	if ( d3dDebugController )
	{
		d3dDebugController->EnableDebugLayer();
		d3dDebugController->Release();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hResult = ::CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &this->dxgiFactory ) );
	IDXGIAdapter1* d3dAdapter = nullptr;
	for ( size_t i = 0; i < DXGI_ERROR_NOT_FOUND != this->dxgiFactory->EnumAdapters1( i, &d3dAdapter ); i++ )
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		d3dAdapter->GetDesc1( &dxgiAdapterDesc );
		if ( dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG::DXGI_ADAPTER_FLAG_SOFTWARE )
		{
			continue;
		}
		if ( SUCCEEDED( ::D3D12CreateDevice( d3dAdapter, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS( &this->d3dDevice ) ) ) )
		{
			DebugNormalMessage( "Init as Adapter : " << dxgiAdapterDesc.Description );
			break;
		}
	}
	if ( !d3dAdapter )
	{
		this->dxgiFactory->EnumWarpAdapter( IID_PPV_ARGS( &d3dAdapter ) );
		D3D12CreateDevice( d3dAdapter, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &this->d3dDevice ) );
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;

	this->d3dDevice->CheckFeatureSupport( D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof( decltype(d3dMsaaQualityLevels) ) );
	this->setting.msaa4xQualityLevel = std::min( this->setting.msaa4xQualityLevel, d3dMsaaQualityLevels.NumQualityLevels );
	this->setting.msaa4xEnable = this->setting.msaa4xEnable & (this->setting.msaa4xQualityLevel > 1);

	hResult = this->d3dDevice->CreateFence( 0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &this->fence ) );
	this->hFenceEvent = ::CreateEvent( nullptr, false, false, nullptr );
	this->fenceValue = 0;
	TryRelease( d3dAdapter );
	;
}

void KG::Renderer::KGDXRenderer::CreateSwapChain()
{
	RECT clientRect;
	::GetClientRect( this->desc.hWnd, &clientRect );
	this->setting.clientWidth = clientRect.right - clientRect.left;
	this->setting.clientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory( &dxgiSwapChainDesc, sizeof( decltype(dxgiSwapChainDesc) ) );
	dxgiSwapChainDesc.Width = this->setting.clientWidth;
	dxgiSwapChainDesc.Height = this->setting.clientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (this->setting.msaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (this->setting.msaa4xEnable) ? (this->setting.msaa4xQualityLevel - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = this->setting.maxSwapChainCount;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory( &dxgiSwapChainFullScreenDesc, sizeof( decltype(dxgiSwapChainFullScreenDesc) ) );
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = true;

	this->dxgiFactory->CreateSwapChainForHwnd( this->commandQueue, this->desc.hWnd,
		&dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc,
		nullptr, (IDXGISwapChain1**)(&this->swapChain) );

	this->dxgiFactory->MakeWindowAssociation( this->desc.hWnd, DXGI_MWA_NO_ALT_ENTER );
	this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
}

void KG::Renderer::KGDXRenderer::CreateRtvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc( d3dDescriptorHeapDesc );

	d3dDescriptorHeapDesc.NumDescriptors = this->setting.maxSwapChainCount;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = this->d3dDevice->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->rtvDescriptorHeap ) );
	this->rtvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
}

void KG::Renderer::KGDXRenderer::CreateDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc( d3dDescriptorHeapDesc );

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = this->d3dDevice->CreateDescriptorHeap( &d3dDescriptorHeapDesc, IID_PPV_ARGS( &this->dsvDescriptorHeap ) );
	this->dsvDescriptoSize = this->d3dDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
}

void KG::Renderer::KGDXRenderer::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	ZeroDesc( d3dCommandQueueDesc );
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hResult = this->d3dDevice->CreateCommandQueue( &d3dCommandQueueDesc, IID_PPV_ARGS( &this->commandQueue ) );
	hResult = this->d3dDevice->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &this->mainCommandAllocator ) );
	hResult = this->d3dDevice->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, this->mainCommandAllocator, NULL, IID_PPV_ARGS( &this->mainCommandList ) );
	hResult = this->mainCommandList->Close();
}

void KG::Renderer::KGDXRenderer::CreateRenderTargetView()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetBuffers.resize( this->setting.maxSwapChainCount );
	for ( size_t i = 0; i < this->setting.maxSwapChainCount; i++ )
	{
		this->swapChain->GetBuffer( i, IID_PPV_ARGS( &this->renderTargetBuffers[i] ) );
		this->d3dDevice->CreateRenderTargetView( this->renderTargetBuffers[i], nullptr, rtvCpuDescHandle );
		rtvCpuDescHandle.ptr += this->rtvDescriptorSize;
	}
}

void KG::Renderer::KGDXRenderer::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroDesc( d3dResourceDesc );
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = this->setting.clientWidth;
	d3dResourceDesc.Height = this->setting.clientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = this->setting.msaa4xEnable ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = this->setting.msaa4xEnable ? this->setting.msaa4xQualityLevel - 1 : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	ZeroDesc( d3dHeapProperties );
	d3dHeapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	this->d3dDevice->CreateCommittedResource( &d3dHeapProperties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, IID_PPV_ARGS( &this->depthStencilBuffer ) );

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	this->d3dDevice->CreateDepthStencilView( this->depthStencilBuffer, nullptr, d3dDsvCPUDescriptorHandle );
}

void KG::Renderer::KGDXRenderer::CreateGeneralRootSignature()
{
	D3D12_ROOT_PARAMETER pd3dRootParameters[5]{};

	// 0 : Space 0 : CBV 0 : Camera Data
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 1 : Space 0 : CBV 1 : Pass Data

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 2 : Space 0 : SRV 0 : Instance Data
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;


	// 3 : Space 1 : SRV 0 : Texture Data1 // unbounded

	D3D12_DESCRIPTOR_RANGE txtureData1Range;
	ZeroDesc( txtureData1Range );
	txtureData1Range.BaseShaderRegister = 0;
	txtureData1Range.NumDescriptors = -1;
	txtureData1Range.OffsetInDescriptorsFromTableStart = 0;
	txtureData1Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	txtureData1Range.RegisterSpace = 1;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &txtureData1Range;
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	// 4 : Space 2 : SRV 0 : Texture Data2 // unbounded

	D3D12_DESCRIPTOR_RANGE txtureData2Range;
	ZeroDesc( txtureData2Range );
	txtureData2Range.BaseShaderRegister = 0;
	txtureData2Range.NumDescriptors = -1;
	txtureData2Range.OffsetInDescriptorsFromTableStart = 0;
	txtureData2Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	txtureData2Range.RegisterSpace = 2;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &txtureData2Range;
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;

	ZeroDesc( d3dRootSignatureDesc );
	d3dRootSignatureDesc.NumParameters = _countof( pd3dRootParameters );
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = nullptr;
	ID3DBlob* pd3dErrorBlob = nullptr;

	auto result = ::D3D12SerializeRootSignature( &d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob );
	this->d3dDevice->CreateRootSignature( 0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS( &this->generalRootSignature ) );
	TryRelease( pd3dSignatureBlob );
	TryRelease( pd3dErrorBlob );
}

void KG::Renderer::KGDXRenderer::MoveToNextFrame()
{
	this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
	const UINT64 fenceValue = ++this->fenceValue;
	HRESULT hResult = this->commandQueue->Signal( this->fence, fenceValue );
	if ( FAILED( hResult ) )
	{
		throw hResult;
	}
	if ( this->fence->GetCompletedValue() < fenceValue )
	{
		hResult = this->fence->SetEventOnCompletion( fenceValue, this->hFenceEvent );
		::WaitForSingleObject( this->hFenceEvent, INFINITE );
	}
}