#include "pch.h"
#include "KGDXRenderer.h"
#include "Debug.h"
#include "D3D12Helper.h"
#include <string>
#include <sstream>

using namespace KG::Renderer;
KG::Renderer::KGDXRenderer::KGDXRenderer()
{
}

void KGDXRenderer::Initialize()
{
	this->CreateD3DDevice();
	this->CreateCommandQueueAndList();
	this->CreateSwapChain();
	this->CreateRtvDescriptorHeaps();
	this->CreateDsvDescriptorHeaps();
	this->CreateRenderTargetView();
	this->CreateDepthStencilView();
}

void KGDXRenderer::Render()
{
}

void KG::Renderer::KGDXRenderer::Update()
{
	NotImplement(KGDXRenderer::Update);
}

void KGDXRenderer::OnChangeSettings(const RendererSetting& prev, const RendererSetting& next)
{
	NotImplement(KGDXRenderer::OnChangeSettings);
}

void KG::Renderer::KGDXRenderer::CreateD3DDevice()
{
	HRESULT hResult;
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) | defined(DEBUF)
	ID3D12Debug* d3dDebugController = nullptr;
	hResult = D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebugController));
	if (d3dDebugController)
	{
		d3dDebugController->EnableDebugLayer();
		d3dDebugController->Release();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hResult = ::CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&this->dxgiFactory));
	IDXGIAdapter1* d3dAdapter = nullptr;
	for (size_t i = 0; i < DXGI_ERROR_NOT_FOUND != this->dxgiFactory->EnumAdapters1(i, &d3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		d3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG::DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}
		if (SUCCEEDED(::D3D12CreateDevice(d3dAdapter, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&this->d3dDevice))))
		{
			std::wstringstream wss;
			wss << L"KGFrameWork : Init as Adapter : ";
			wss << dxgiAdapterDesc.Description << std::endl;;
			DebugMessage(wss.str().data());
			break;
		}
	}
	if (!d3dAdapter)
	{
		this->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&d3dAdapter));
		D3D12CreateDevice(d3dAdapter, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->d3dDevice));
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;

	this->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(decltype(d3dMsaaQualityLevels)));
	this->setting.msaa4xQualityLevel = std::min(this->setting.msaa4xQualityLevel, d3dMsaaQualityLevels.NumQualityLevels);
	this->setting.msaa4xEnable = this->setting.msaa4xEnable & (this->setting.msaa4xQualityLevel > 1);

	hResult = this->d3dDevice->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence));
	this->hFenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
	this->fenceValue = 0;
	TryRelease(d3dAdapter);
;}

void KG::Renderer::KGDXRenderer::CreateSwapChain()
{
	RECT clientRect;
	::GetClientRect(this->desc.hWnd, &clientRect);
	this->setting.clientWidth= clientRect.right - clientRect.left;
	this->setting.clientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(decltype(dxgiSwapChainDesc)));
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
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(decltype(dxgiSwapChainFullScreenDesc)));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = true;

	this->dxgiFactory->CreateSwapChainForHwnd(this->commandQueue, this->desc.hWnd,
		&dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc,
		nullptr, (IDXGISwapChain1**)(&this->swapChain));

	this->dxgiFactory->MakeWindowAssociation(this->desc.hWnd, DXGI_MWA_NO_ALT_ENTER);
	this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
}

void KG::Renderer::KGDXRenderer::CreateRtvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc(d3dDescriptorHeapDesc);

	d3dDescriptorHeapDesc.NumDescriptors = this->setting.maxSwapChainCount;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = this->d3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&this->rtvDescriptorHeap));
	this->rtvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void KG::Renderer::KGDXRenderer::CreateDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	ZeroDesc(d3dDescriptorHeapDesc);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = this->d3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, IID_PPV_ARGS(&this->dsvDescriptorHeap));
	this->dsvDescriptoSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void KG::Renderer::KGDXRenderer::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	ZeroDesc(d3dCommandQueueDesc);
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hResult = this->d3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, IID_PPV_ARGS(&this->commandQueue));
	hResult = this->d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->mainCommandAllocator));
	hResult = this->d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, this->mainCommandAllocator, NULL, IID_PPV_ARGS(&this->mainCommandList));
	hResult = this->mainCommandList->Close();
}

void KG::Renderer::KGDXRenderer::CreateRenderTargetView()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuDescHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	renderTargetBuffers.resize(this->setting.maxSwapChainCount);
	for (size_t i = 0; i < this->setting.maxSwapChainCount; i++)
	{
		this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargetBuffers[i]));
		this->d3dDevice->CreateRenderTargetView(this->renderTargetBuffers[i], nullptr, rtvCpuDescHandle);
		rtvCpuDescHandle.ptr += this->rtvDescriptorSize;
	}
}

void KG::Renderer::KGDXRenderer::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	ZeroDesc(d3dResourceDesc);
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
	ZeroDesc(d3dHeapProperties);
	d3dHeapProperties.Type = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY::D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL::D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	this->d3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, IID_PPV_ARGS(&this->depthStencilBuffer));

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = this->dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	this->d3dDevice->CreateDepthStencilView(this->depthStencilBuffer, nullptr, d3dDsvCPUDescriptorHandle);
}
