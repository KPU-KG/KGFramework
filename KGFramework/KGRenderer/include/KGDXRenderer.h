#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGRenderer.h"
#include "KGShader.h"
#include "ParticleGenerator.h"
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

		UINT rtvDescriptorSize = 0;
		UINT srvDescriptorSize = 0;
		UINT dsvDescriptoSize = 0;

		ID3D12CommandQueue* commandQueue = nullptr;
		ID3D12CommandAllocator* mainCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* mainCommandList = nullptr;

		ID3D12RootSignature* generalRootSignature = nullptr;

		ID3D12Fence* fence = nullptr;
		UINT64 fenceValue = 0;
		HANDLE hFenceEvent = 0;

		bool isWireFrame = false;
		
		size_t imguiFontDescIndex = 0;

		double gameTime = 0.0f;


		struct GraphicSystems;
		std::unique_ptr<GraphicSystems> graphicSystems = nullptr;
		std::unique_ptr<KGRenderEngine> renderEngine = nullptr;
		std::unique_ptr<DescriptorHeapManager> descriptorHeapManager = nullptr;
		static inline KGDXRenderer* instance = nullptr;

		//ParticleBuffer
		ParticleGenerator particleGenerator;

	private:
		void QueryHardwareFeature();

		void CreateD3DDevice();
		void CreateSwapChain();
		void CreateRtvDescriptorHeaps();
		void CreateSRVDescriptorHeaps();
		void CreateCommandQueueAndList();
		void CreateRenderTargetView();

		void InitializeImGui();

		void CreateGeneralRootSignature();

		void AllocateGBufferHeap();

		void MoveToNextFrame();

		D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetHandle() const;
		ID3D12Resource* GetCurrentRenderTarget() const;
	public:
		KGDXRenderer();
		~KGDXRenderer();
		KGDXRenderer(const KGDXRenderer&) = delete;
		KGDXRenderer(KGDXRenderer&&) = delete;
		KGDXRenderer& operator=(const KGDXRenderer&) = delete;
		KGDXRenderer& operator=(KGDXRenderer&&) = delete;



		virtual void Initialize() override;
		virtual void SetGameTime(double gameTime) override;
		virtual void Render() override;
		virtual void PreRenderUI() override;
		virtual void PreloadModels(std::vector<KG::Utill::HashString>&& ids) override;
		virtual void CubeCaemraRender();
		virtual void NormalCameraRender();
		virtual void ShadowMapRender();
		virtual void CopyMainCamera();
		virtual void UIRender();
		virtual void OpaqueRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
		virtual void TransparentRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
		virtual void ParticleRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
		virtual void LightPassRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
		virtual void SkyBoxRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
		virtual void PassRenderEnd(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);

		void EmitParticle(const KG::Component::ParticleDesc& particleDesc, bool autofillTime);

		virtual void Update(float elapsedTime) override;
		virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;

		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

		virtual void* GetImGUIContext();
		virtual KG::Component::Render3DComponent* GetNewRenderComponent() override;
		virtual KG::Component::GeometryComponent* GetNewGeomteryComponent() override;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponent() override;
		virtual KG::Component::CameraComponent* GetNewCameraComponent() override;
		virtual KG::Component::CubeCameraComponent* GetNewCubeCameraComponent() override;
		virtual KG::Component::LightComponent* GetNewLightComponent() override;
		virtual KG::Component::ShadowCasterComponent* GetNewShadowCasterComponent() override;
		virtual KG::Component::BoneTransformComponent* GetNewBoneTransformComponent() override;
		virtual KG::Component::AnimationControllerComponent* GetNewAnimationControllerComponent() override;
		virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials) override;
		virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials) override;


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


		// IKGRenderer을(를) 통해 상속됨
		virtual double GetGameTime() const override;


		// IKGRenderer을(를) 통해 상속됨
		virtual UINT QueryMaterialIndex(const KG::Utill::HashString& materialId) const override;

};
}