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
    namespace RTX
    {
        class KGRTXSubRenderer;
    };
    struct RenderTexture;
    using std::vector;

    constexpr UINT maxFrameCount = 3;
    struct HardwareFeature
    {
        UINT rtvDescriptorSize = 0;
        UINT srvDescriptorSize = 0;
        UINT dsvDescriptorSize = 0;
        UINT frameResourceCount = maxFrameCount;
        bool isAbleDXR = false;
    };
    class KGDXRenderer : public IKGRenderer
    {
        friend class RTX::KGRTXSubRenderer;
    private:
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain3* swapChain = nullptr;

        ID3D12Device* d3dDevice = nullptr;

        UINT swapChainBufferIndex = 0;

        RTX::KGRTXSubRenderer* dxrRenderer;

        //vector<ID3D12Resource*> renderTargetBuffers;
        //ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
        vector<KG::Resource::DXResource> renderTargetResources;
        DescriptorHeapManager rtvDescriptorHeap;

        HardwareFeature hwFeature;

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
        bool rtxOn = true;
        bool rtxMain = true;


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

        void CreateDXR();

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

        void Initialize() override;
        void SetGameTime(double gameTime) override;
        void Render() override;
        void PreRenderEditorUI() override;
        void PreloadModels(std::vector<KG::Utill::HashString>&& ids) override;
        void CubeCaemraRender();
        void NormalCameraRender();
        void ShadowMapRender();
        void CopyMainCamera();
        void EditorUIRender();
        void ParticleReady();
        void OpaqueRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, bool culled = false);
        void TransparentRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void ParticleRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void SpriteRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void InGameUIRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void SSAORender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, ID3D12Resource* data);
        void LightPassRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void SkyBoxRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);
        void PostProcessRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, ID3D12Resource* data);
        void PassRenderEnd(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex);

        ParticleGenerator* GetParticleGenerator();

        virtual void Update(float elapsedTime) override;
        virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;

        virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

        virtual void DebugUIRender() override;
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

        RendererSetting GetSetting() const;
        ID3D12Device* GetD3DDevice() const;
        UINT64 GetFenceValue() const;
        KG::Renderer::PostProcessor* GetPostProcess() const;
        KG::Renderer::RTX::KGRTXSubRenderer* GetRayRender() const;
        bool isRayRender() const;
        static KGDXRenderer* GetInstance();
        ID3D12RootSignature* GetGeneralRootSignature() const;
        ID3D12RootSignature* GetPostProcessRootSignature() const;
        KGRenderEngine* GetRenderEngine() const;
        DescriptorHeapManager* GetDescriptorHeapManager() const;
        
        const HardwareFeature& GetHWFeature() const;

        // IKGRenderer을(를) 통해 상속됨
        virtual double GetGameTime() const override;

        // IKGRenderer을(를) 통해 상속됨
        virtual UINT QueryMaterialIndex(const KG::Utill::HashString& materialId) const override;

        // IKGRenderer을(를) 통해 상속됨
        virtual void SetEditUIRender(bool isRender) override;
    };
}