#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGRenderer.h"
namespace KG::Renderer
{
	class KGRenderEngine;
	class DescriptorHeapManager;
	using std::vector;
	class KGDXRenderer : public IKGRenderer
	{
	private:
		IDXGIFactory4* dxgiFactory = nullptr;
		IDXGISwapChain3* swapChain = nullptr;

		ID3D12Device* d3dDevice = nullptr;

		UINT swapChainBufferIndex = 0;

		vector<ID3D12Resource*> renderTargetBuffers;
		ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

		vector<ID3D12Resource*> gbufferTargetBuffers;
		ID3D12DescriptorHeap* gbufferDescriptorHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE gbufferDescritorHandles[4];
		D3D12_RESOURCE_BARRIER gbufferResourceBarrierCache[4];

		UINT rtvDescriptorSize = 0;
		UINT srvDescriptorSize = 0;

		ID3D12Resource* depthStencilBuffer = nullptr;
		ID3D12Resource* depthStencilBackBuffer = nullptr;
		ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
		UINT dsvDescriptoSize = 0;
		
		ID3D12CommandQueue* commandQueue = nullptr;
		ID3D12CommandAllocator* mainCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* mainCommandList = nullptr;

		ID3D12RootSignature* generalRootSignature = nullptr;

		ID3D12Fence* fence = nullptr;
		UINT64 fenceValue = 0;
		HANDLE hFenceEvent = 0;

		bool isWireFrame = false;

		struct GraphicSystems;
		std::unique_ptr<GraphicSystems> graphicSystems = nullptr;
		std::unique_ptr<KGRenderEngine> renderEngine = nullptr;
		std::unique_ptr<DescriptorHeapManager> descriptorHeapManager = nullptr;
		static inline KGDXRenderer* instance = nullptr;
	private:
		void QueryHardwareFeature();

		void CreateD3DDevice();
		void CreateSwapChain();
		void CreateRtvDescriptorHeaps();
		void CreateDsvDescriptorHeaps();
		void CreateSRVDescriptorHeaps();
		void CreateCommandQueueAndList();
		void CreateRenderTargetView();
		void CreateDepthStencilView();

		void CreateGBuffer();
		void CreateGBufferDescriptorHeaps();

		void CreateGeneralRootSignature();

		void CreateGbufferSRV();
		
		void RegisterPassEnterFunction();

		void MoveToNextFrame();

		D3D12_RESOURCE_BARRIER* GetGBufferTrasitionBarriers( D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

		D3D12_RESOURCE_BARRIER* GetLightBufferTrasitionBarriers
		( D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after,
			D3D12_RESOURCE_STATES lightBefore, D3D12_RESOURCE_STATES lightAfter );

		void ClearGBuffer( ID3D12GraphicsCommandList* cmdList, float r, float g, float b, float a );

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetHandle() const;
		D3D12_CPU_DESCRIPTOR_HANDLE& GetCurrentDepthStencilHandle() const;
		ID3D12Resource* GetCurrentRenderTarget() const;
	public:
		KGDXRenderer();
		~KGDXRenderer();
		KGDXRenderer( const KGDXRenderer& ) = delete;
		KGDXRenderer( KGDXRenderer&& ) = delete;
		KGDXRenderer& operator=( const KGDXRenderer& ) = delete;
		KGDXRenderer& operator=( KGDXRenderer&& ) = delete;



		virtual void Initialize() override;
		virtual void Render() override;
		virtual void Update(float elapsedTime) override;
		virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;

		virtual KG::Component::Render3DComponent* GetNewRenderComponent() override;
		virtual KG::Component::GeometryComponent* GetNewGeomteryComponent( const KG::Utill::HashString& id ) override;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponent( const KG::Utill::HashString& id ) override;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponentFromShader( const KG::Utill::HashString& id ) override;
		virtual KG::Component::CameraComponent* GetNewCameraComponent() override;
		virtual KG::Component::LightComponent* GetNewLightComponent() override;

		auto GetD3DDevice() const
		{
			return this->d3dDevice;
		};
		auto GetFenceValue() const
		{
			return this->fenceValue;
		};

		static KGDXRenderer* GetInstance();
		ID3D12RootSignature* GetGeneralRootSignature() const;
		KGRenderEngine* GetRenderEngine() const;
		DescriptorHeapManager* GetDescriptorHeapManager() const;
		UINT GetRTVSize() const
		{
			return this->rtvDescriptorSize;
		};
		UINT GetSRVSize() const 
		{
			return this->srvDescriptorSize;
		};
		UINT GetDSVSize() const
		{
			return this->dsvDescriptoSize;
		};

	};
}