#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGDX12Resource.h"
#include "KGRenderer.h"
#include "KGShader.h"
#include "ParticleGenerator.h"
namespace KG::Renderer
{
    class KGRenderEngine;
    class PostProcessor;
    class DescriptorHeapManager;
    struct RenderTexture;
    using std::vector;
    class KGDXRenderer : public IKGRenderer
    {
    private:
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain3* swapChain = nullptr;

        ID3D12Device* d3dDevice = nullptr;

        UINT swapChainBufferIndex = 0;

        //vector<ID3D12Resource*> renderTargetBuffers;
        //ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
        vector<KG::Resource::DXResource> renderTargetResources;
        DescriptorHeapManager rtvDescriptorHeap;

        UINT rtvDescriptorSize = 0;
        UINT srvDescriptorSize = 0;
        UINT dsvDescriptoSize = 0;

        ID3D12CommandQueue* commandQueue = nullptr;
        ID3D12CommandAllocator* mainCommandAllocator = nullptr;
        ID3D12GraphicsCommandList* mainCommandList = nullptr;

        ID3D12RootSignature* generalRootSignature = nullptr;
        ID3D12RootSignature* postProcessRootSignature = nullptr;

        ID3D12Fence* fence = nullptr;
        UINT64 fenceValue = 0;
        HANDLE hFenceEvent = 0;

        bool isWireFrame = false;
        bool isRenderEditUI = false;

        size_t imguiFontDescIndex = 0;

        double gameTime = 0.0f;

        struct GraphicSystems;
        std::unique_ptr<GraphicSystems> graphicSystems = nullptr;
        std::unique_ptr<KGRenderEngine> renderEngine = nullptr;
        std::unique_ptr<PostProcessor> postProcessor;
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
        void CreatePostProcessRootSignature();

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
        virtual void PreRenderEditorUI() override;
        virtual void PreloadModels(std::vector<KG::Utill::HashString>&& ids) override;
        virtual void CubeCaemraRender();
        virtual void NormalCameraRender();
        virtual void ShadowMapRender();
        virtual void CopyMainCamera();
        virtual void EditorUIRender();
        virtual void OpaqueRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void TransparentRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void ParticleRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void SpriteRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void InGameUIRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void LightPassRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void SkyBoxRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void PostProcessRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        virtual void PassRenderEnd(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);

        ParticleGenerator* GetParticleGenerator();

        virtual void Update(float elapsedTime) override;
        virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;

        virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

        virtual void* GetImGUIContext();
        virtual KG::Component::IRender3DComponent* GetNewRenderComponent() override;
        virtual KG::Component::IRender2DComponent* GetNewRender2DComponent() override;
        virtual KG::Component::IRenderSpriteComponent* GetNewRenderSpriteComponent() override;
        virtual KG::Component::IGeometryComponent* GetNewGeomteryComponent() override;
        virtual KG::Component::IMaterialComponent* GetNewMaterialComponent() override;
        virtual KG::Component::ICameraComponent* GetNewCameraComponent() override;
        virtual KG::Component::ICubeCameraComponent* GetNewCubeCameraComponent() override;
        virtual KG::Component::ILightComponent* GetNewLightComponent() override;
        virtual KG::Component::IShadowCasterComponent* GetNewShadowCasterComponent() override;
        virtual KG::Component::IBoneTransformComponent* GetNewBoneTransformComponent() override;
        virtual KG::Component::IAnimationControllerComponent* GetNewAnimationControllerComponent() override;
        virtual KG::Component::IParticleEmitterComponent* GetNewParticleEmitterComponent() override;
        virtual KG::Component::IPostProcessManagerComponent* GetNewPostProcessorComponent() override;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials) override;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials) override;

        auto GetSetting() const
        {
            return this->setting;
        }

        auto GetD3DDevice() const
        {
            return this->d3dDevice;
        };
        auto GetFenceValue() const
        {
            return this->fenceValue;
        };

        auto GetPostProcess() const
        {
            return this->postProcessor.get();
        };

        static KGDXRenderer* GetInstance();
        ID3D12RootSignature* GetGeneralRootSignature() const;
        ID3D12RootSignature* GetPostProcessRootSignature() const;
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

        // IKGRenderer을(를) 통해 상속됨
        virtual void SetEditUIRender(bool isRender) override;
    };
}