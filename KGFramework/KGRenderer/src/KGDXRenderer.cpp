#include "pch.h"
#include "KGDXRenderer.h"
#include "Debug.h"
#include "D3D12Helper.h"
#include <string>
#include <sstream>
#include "KGRenderQueue.h"
#include "GraphicSystem.h"
#include "ResourceContainer.h"
#include "RootParameterIndex.h"
#include "DescriptorHeapManager.h"
#include "d3dx12.h"
#include "Texture.h"


using namespace KG::Renderer;

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

struct KG::Renderer::KGDXRenderer::GraphicSystems
{
	KG::System::Render3DSystem render3DSystem;
	KG::System::GeometrySystem geometrySystem;
	KG::System::MaterialSystem materialSystem;
	KG::System::CameraSystem cameraSystem;
	KG::System::CubeCameraSystem cubeCameraSystem;
	KG::System::LightSystem lightSystem;

	void OnPreRender()
	{
		this->geometrySystem.OnPreRender();
		this->materialSystem.OnPreRender();
		this->render3DSystem.OnPreRender();
		this->cameraSystem.OnPreRender();
		this->cubeCameraSystem.OnPreRender();
		this->lightSystem.OnPreRender();
	}

	void OnUpdate( float elaspedTime )
	{
		this->geometrySystem.OnUpdate( elaspedTime );
		this->materialSystem.OnUpdate( elaspedTime );
		this->render3DSystem.OnUpdate( elaspedTime );
		this->cameraSystem.OnUpdate( elaspedTime );
		this->cubeCameraSystem.OnUpdate( elaspedTime );
		this->lightSystem.OnUpdate( elaspedTime );
	}
	void OnPostUpdate( float elaspedTime )
	{
		this->geometrySystem.OnPostUpdate( elaspedTime );
		this->materialSystem.OnPostUpdate( elaspedTime );
		this->render3DSystem.OnPostUpdate( elaspedTime );
		this->cameraSystem.OnPostUpdate( elaspedTime );
		this->cubeCameraSystem.OnPostUpdate( elaspedTime );
		this->lightSystem.OnPostUpdate( elaspedTime );
	}

	void Clear()
	{
		this->geometrySystem.Clear();
		this->materialSystem.Clear();
		this->render3DSystem.Clear();
		this->cameraSystem.Clear();
		this->cubeCameraSystem.Clear();
		this->lightSystem.Clear();
	}
};


KG::Renderer::KGDXRenderer::KGDXRenderer()
{
	KG::Resource::ResourceContainer::GetInstance();
}

KG::Renderer::KGDXRenderer::~KGDXRenderer()
{
	KG::Resource::ResourceContainer::GetInstance()->Clear();
	this->graphicSystems->Clear();
	TryRelease( dxgiFactory );
	TryRelease( swapChain );
	TryRelease( d3dDevice );
	TryRelease( rtvDescriptorHeap );
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
	DebugNormalMessage( "RECT : " << this->setting.clientWidth << " , " << this->setting.clientHeight );

	this->CreateD3DDevice();
	this->QueryHardwareFeature();
	this->CreateCommandQueueAndList();
	this->CreateSwapChain();
	this->CreateRtvDescriptorHeaps();
	this->CreateRenderTargetView();
	this->CreateGeneralRootSignature();


	this->renderEngine = std::make_unique<KGRenderEngine>( this->d3dDevice );
	this->graphicSystems = std::make_unique<GraphicSystems>();

	this->CreateSRVDescriptorHeaps();
	this->AllocateGBufferHeap();

	this->RegisterPassEnterFunction();
}

void KGDXRenderer::Render()
{
	this->graphicSystems->OnPreRender();

	HRESULT hResult = this->mainCommandAllocator->Reset();
	hResult = this->mainCommandList->Reset( this->mainCommandAllocator, nullptr );

	KG::Resource::ResourceContainer::GetInstance()->Process( this->mainCommandList );

	this->mainCommandList->SetGraphicsRootSignature( this->generalRootSignature );
	ID3D12DescriptorHeap* heaps[] = { this->descriptorHeapManager->Get() };
	this->mainCommandList->SetDescriptorHeaps( 1, heaps );

	PIXBeginEvent( mainCommandList, PIX_COLOR_INDEX( 0 ), "CubeCameraRender" );
	for ( KG::Component::CubeCameraComponent& cubeCamera : this->graphicSystems->cubeCameraSystem )
	{
		for ( KG::Component::CameraComponent& camera : cubeCamera.GetCameras() )
		{
			PIXBeginEvent( mainCommandList, PIX_COLOR_INDEX( 0 ), "Cube Camera Render : Camera %d", camera.GetCubeIndex() );
			camera.SetCameraRender( this->mainCommandList );
			this->renderEngine->Render( this->mainCommandList, camera.GetRenderTexture() );
			camera.EndCameraRender( this->mainCommandList );
			PIXEndEvent( mainCommandList );
		}
		TryResourceBarrier( this->mainCommandList,
			cubeCamera.GetRenderTexture().BarrierTransition(
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_COMMON,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			)
		);
	}
	PIXEndEvent( mainCommandList );

	PIXBeginEvent( mainCommandList, PIX_COLOR_INDEX( 1 ), "NormalCameraRender" );
	size_t _cameraCount = 1;
	for ( KG::Component::CameraComponent& camera : this->graphicSystems->cameraSystem )
	{
		PIXBeginEvent( mainCommandList, PIX_COLOR_INDEX( 1 ), "Normal Camera Render : Camera %d", _cameraCount++ );
		camera.SetCameraRender( this->mainCommandList );
		this->renderEngine->Render( this->mainCommandList, camera.GetRenderTexture());
		camera.EndCameraRender( this->mainCommandList );

		if ( camera.isMainCamera )
		{
			TryResourceBarrier( this->mainCommandList,
				camera.GetRenderTexture().BarrierTransition(
					D3D12_RESOURCE_STATE_COPY_SOURCE,
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COMMON
					)
				);

			this->mainCommandList->ResourceBarrier( 1, 
				&CD3DX12_RESOURCE_BARRIER::Transition( 
					this->renderTargetBuffers[this->swapChainBufferIndex],
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_COPY_DEST )
			);
			this->mainCommandList->CopyResource( this->renderTargetBuffers[this->swapChainBufferIndex], camera.GetRenderTexture().renderTarget );
			this->mainCommandList->ResourceBarrier( 1,
				&CD3DX12_RESOURCE_BARRIER::Transition(
					this->renderTargetBuffers[this->swapChainBufferIndex],
					D3D12_RESOURCE_STATE_COPY_DEST,
					D3D12_RESOURCE_STATE_PRESENT )
			);

			TryResourceBarrier( this->mainCommandList,
				camera.GetRenderTexture().BarrierTransition(
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_COMMON
				)
			);
		}
		PIXEndEvent( mainCommandList );

	}
	PIXEndEvent( mainCommandList );

	hResult = this->mainCommandList->Close();

	ID3D12CommandList* d3dCommandLists[] = { this->mainCommandList };
	this->commandQueue->ExecuteCommandLists( 1, d3dCommandLists );

	this->renderEngine->ClearUpdateCount();

	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = nullptr;
	dxgiPresentParameters.pScrollRect = nullptr;
	dxgiPresentParameters.pScrollOffset = nullptr;
	this->swapChain->Present1( this->setting.isVsync, 0, &dxgiPresentParameters );

	this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();

	this->MoveToNextFrame();
}

void KG::Renderer::KGDXRenderer::Update( float elapsedTime )
{
	this->graphicSystems->OnUpdate( elapsedTime );
	this->graphicSystems->OnPostUpdate( elapsedTime );
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
	mat->InitializeMaterial( id );
	return mat;
}

KG::Component::MaterialComponent* KG::Renderer::KGDXRenderer::GetNewMaterialComponentFromShader( const KG::Utill::HashString& id )
{
	auto* mat = static_cast<KG::Component::MaterialComponent*>(this->graphicSystems->materialSystem.GetNewComponent());
	mat->InitializeShader( id );
	return mat;
}

KG::Component::CameraComponent* KG::Renderer::KGDXRenderer::GetNewCameraComponent()
{
	return static_cast<KG::Component::CameraComponent*>(this->graphicSystems->cameraSystem.GetNewComponent());
}

KG::Component::CubeCameraComponent* KG::Renderer::KGDXRenderer::GetNewCubeCameraComponent()
{
	return static_cast<KG::Component::CubeCameraComponent*>(this->graphicSystems->cubeCameraSystem.GetNewComponent());
}

KG::Component::LightComponent* KG::Renderer::KGDXRenderer::GetNewLightComponent()
{
	return static_cast<KG::Component::LightComponent*>(this->graphicSystems->lightSystem.GetNewComponent());
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

DescriptorHeapManager* KG::Renderer::KGDXRenderer::GetDescriptorHeapManager() const
{
	return this->descriptorHeapManager.get();
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

	if ( featureSupport.PSSpecifiedStencilRefSupported )
	{
		DebugNormalMessage( "D3D12 Stencil Ref Supported" );
	}
	else 
	{
		DebugNormalMessage( "D3D12 Stencil Ref Not Supported" );
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

void KG::Renderer::KGDXRenderer::CreateSRVDescriptorHeaps()
{
	this->descriptorHeapManager = std::make_unique<DescriptorHeapManager>();
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	// ! 서술자 갯수
	srvHeapDesc.NumDescriptors = 100;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	this->srvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	this->descriptorHeapManager->Initialize( this->d3dDevice, srvHeapDesc, this->srvDescriptorSize );

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


void KG::Renderer::KGDXRenderer::CreateGeneralRootSignature()
{
	D3D12_ROOT_PARAMETER pd3dRootParameters[7]{};

	// 0 : Space 0 : SRV 0 : Instance Data : 2
	pd3dRootParameters[RootParameterIndex::InstanceData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[RootParameterIndex::InstanceData].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[RootParameterIndex::InstanceData].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[RootParameterIndex::InstanceData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

	// 1 : Space 1 : CBV 0 : Material Data : 2
	pd3dRootParameters[RootParameterIndex::MaterialData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[RootParameterIndex::MaterialData].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[RootParameterIndex::MaterialData].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[RootParameterIndex::MaterialData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	// 2 : Space 0 : CBV 0 : Camera Data : 2
	pd3dRootParameters[RootParameterIndex::CameraData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[RootParameterIndex::CameraData].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[RootParameterIndex::CameraData].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[RootParameterIndex::CameraData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

	// 3 : Space 0 : CBV 1 : Pass Data : 2
	pd3dRootParameters[RootParameterIndex::PassData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[RootParameterIndex::PassData].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[RootParameterIndex::PassData].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[RootParameterIndex::PassData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;


	// 4 : Space 1 : SRV 0 : Texture Data1 // unbounded : 1

	D3D12_DESCRIPTOR_RANGE txtureData1Range;
	ZeroDesc( txtureData1Range );
	txtureData1Range.BaseShaderRegister = 0;
	txtureData1Range.NumDescriptors = -1;
	txtureData1Range.OffsetInDescriptorsFromTableStart = 0;
	txtureData1Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	txtureData1Range.RegisterSpace = 1;

	pd3dRootParameters[RootParameterIndex::Texture1Heap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[RootParameterIndex::Texture1Heap].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[RootParameterIndex::Texture1Heap].DescriptorTable.pDescriptorRanges = &txtureData1Range;
	pd3dRootParameters[RootParameterIndex::Texture1Heap].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	// 5 : Space 2 : SRV 0 : Texture Data2 // unbounded : 1

	D3D12_DESCRIPTOR_RANGE txtureData2Range;
	ZeroDesc( txtureData2Range );
	txtureData2Range.BaseShaderRegister = 0;
	txtureData2Range.NumDescriptors = -1;
	txtureData2Range.OffsetInDescriptorsFromTableStart = 0;
	txtureData2Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	txtureData2Range.RegisterSpace = 2;

	pd3dRootParameters[RootParameterIndex::Texture2Heap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[RootParameterIndex::Texture2Heap].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[RootParameterIndex::Texture2Heap].DescriptorTable.pDescriptorRanges = &txtureData2Range;
	pd3dRootParameters[RootParameterIndex::Texture2Heap].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	//G-BUFFER 

	D3D12_DESCRIPTOR_RANGE GbufferRange;
	ZeroDesc( GbufferRange );
	GbufferRange.BaseShaderRegister = 2;
	GbufferRange.NumDescriptors = 5;
	GbufferRange.OffsetInDescriptorsFromTableStart = 0;
	GbufferRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	GbufferRange.RegisterSpace = 0;

	pd3dRootParameters[RootParameterIndex::GBufferHeap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[RootParameterIndex::GBufferHeap].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[RootParameterIndex::GBufferHeap].DescriptorTable.pDescriptorRanges = &GbufferRange;
	pd3dRootParameters[RootParameterIndex::GBufferHeap].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	auto staticSampler = GetStaticSamplers();

	ZeroDesc( d3dRootSignatureDesc );
	d3dRootSignatureDesc.NumParameters = _countof( pd3dRootParameters );
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = staticSampler.size();
	d3dRootSignatureDesc.pStaticSamplers = staticSampler.data();
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = nullptr;
	ID3DBlob* pd3dErrorBlob = nullptr;

	auto result = ::D3D12SerializeRootSignature( &d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob );
	this->d3dDevice->CreateRootSignature( 0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS( &this->generalRootSignature ) );
	TryRelease( pd3dSignatureBlob );
	TryRelease( pd3dErrorBlob );
}

void KG::Renderer::KGDXRenderer::AllocateGBufferHeap()
{
	for ( size_t i = 0; i < 5; i++ )
	{
		auto index = this->descriptorHeapManager->RequestEmptyIndex();
		DebugNormalMessage( L"디스크립터 힙 G-Buffer용 초기화 : " << index );
	}
}

void KG::Renderer::KGDXRenderer::RegisterPassEnterFunction()
{

	this->renderEngine->SetPassEnterEventFunction( ShaderType::Opaque,
		[this]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{
			PIXSetMarker( cmdList, PIX_COLOR( 255, 0, 0 ), "Opaque Render" );
			TryResourceBarrier( cmdList,
				renderTexture.BarrierTransition(
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_DEPTH_WRITE
				)
			);

			cmdList->OMSetRenderTargets( 4, renderTexture.gbufferHandle.data(), false, &renderTexture.dsvHandle );

			renderTexture.ClearGBuffer( this->mainCommandList, 0, 0, 0, 0 );

			cmdList->SetGraphicsRootDescriptorTable( RootParameterIndex::Texture1Heap, this->descriptorHeapManager->GetGPUHandle( 0 ) );

			this->mainCommandList->ClearDepthStencilView( renderTexture.dsvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );
		} );

	this->renderEngine->SetPassEnterEventFunction( ShaderType::LightPass,
		[this]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{
			PIXSetMarker( cmdList, PIX_COLOR( 255, 0, 0 ), "Light Pass Render" );
			TryResourceBarrier( cmdList,
				renderTexture.BarrierTransition(
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
				)
			);
			cmdList->SetGraphicsRootDescriptorTable( RootParameterIndex::GBufferHeap, this->descriptorHeapManager->GetGPUHandle( renderTexture.gbufferSRVIndex ) );
			cmdList->OMSetRenderTargets( 1, &renderTexture.GetRenderTargetRTVHandle(), true, nullptr );
		} );

	this->renderEngine->SetPassPreRenderEventFunction( ShaderType::LightPass,
		[this]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{

		} );


	this->renderEngine->SetPassEnterEventFunction( ShaderType::Transparent,
		[]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{
			PIXSetMarker( cmdList, PIX_COLOR( 255, 0, 0 ), "Transparent Pass Render" );
		} );

	this->renderEngine->SetPassEnterEventFunction( ShaderType::PostProcess,
		[this]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{
			PIXSetMarker( cmdList, PIX_COLOR( 255, 0, 0 ), "PostProcess Pass Render" );
			cmdList->OMSetRenderTargets( 1, &renderTexture.GetRenderTargetRTVHandle(), true, &renderTexture.dsvHandle );
			//cmdList->ResourceBarrier( 4, this->GetGBufferTrasitionBarriers( D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE ) );
			//cmdList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST ) );

			//cmdList->CopyResource( rt , this->gbufferTargetBuffers[1] );

			//cmdList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( rt, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET ) );
			//cmdList->ResourceBarrier( 4, this->GetGBufferTrasitionBarriers( D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );
		} );

	this->renderEngine->SetPassEndEventFunction(
		[this]( ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture )
		{
			TryResourceBarrier( cmdList,
				renderTexture.BarrierTransition(
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
					D3D12_RESOURCE_STATE_DEPTH_WRITE
				)
			);
		} );

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

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::KGDXRenderer::GetCurrentRenderTargetHandle() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (this->swapChainBufferIndex * this->rtvDescriptorSize);
	return d3dRtvCPUDescriptorHandle;
}

ID3D12Resource* KG::Renderer::KGDXRenderer::GetCurrentRenderTarget() const
{
	return this->renderTargetBuffers[this->swapChainBufferIndex];
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP ); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP ); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP ); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP ); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8 );                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		16 );                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}