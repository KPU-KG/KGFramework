#include "pch.h"
#include "KGDXRenderer.h"
#include "Debug.h"
#include "D3D12Helper.h"
#include "ImguiHelper.h"
#include <string>
#include <sstream>
#include "KGRenderQueue.h"
#include "GraphicSystem.h"
#include "ResourceContainer.h"
#include "RootParameterIndex.h"
#include "DescriptorHeapManager.h"
#include "d3dx12.h"
#include "Texture.h"
#include "KGRTXSubRenderer.h"



using namespace KG::Renderer;

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GetStaticSamplers();

struct KG::Renderer::KGDXRenderer::GraphicSystems
{
    KG::System::Render2DSystem render2DSystem;
    KG::System::Render3DSystem render3DSystem;
    KG::System::RenderSpriteSystem renderSpriteSystem;
    KG::System::GeometrySystem geometrySystem;
    KG::System::MaterialSystem materialSystem;
    KG::System::CameraSystem cameraSystem;
    KG::System::CubeCameraSystem cubeCameraSystem;
    KG::System::LightSystem lightSystem;
    KG::System::AvatarSystem avatarSystem;
    KG::System::AnimationControllerSystem animationControllerSystem;
    KG::System::ShadowCasterSystem shadowSystem;
    KG::System::ParticleEmitterSystem particleSystem;
    KG::System::PostProcessorSystem postProcessorSystem;

    void OnProcessCulling(const DirectX::BoundingFrustum& frustum)
    {
        this->render3DSystem.OnProcessCulling(frustum);
    }

    void OnCameraPreRender()
    {
        this->cameraSystem.OnPreRender();
        this->cubeCameraSystem.OnPreRender();
    }
    void OnPreRender()
    {
        this->geometrySystem.OnPreRender();
        this->materialSystem.OnPreRender();
        this->render2DSystem.OnPreRender();
        this->render3DSystem.OnPreRender();
        this->renderSpriteSystem.OnPreRender();
        //this->cameraSystem.OnPreRender();
        //this->cubeCameraSystem.OnPreRender();
        this->shadowSystem.OnPreRender();
        this->lightSystem.OnPreRender();
        this->avatarSystem.OnPreRender();
        this->animationControllerSystem.OnPreRender();
        this->particleSystem.OnPreRender();
        this->postProcessorSystem.OnPreRender();
    }

    void OnUpdate(float elapsedTime)
    {
        this->geometrySystem.OnUpdate(elapsedTime);
        this->materialSystem.OnUpdate(elapsedTime);
        this->render2DSystem.OnUpdate(elapsedTime);
        this->render3DSystem.OnUpdate(elapsedTime);
        this->renderSpriteSystem.OnUpdate(elapsedTime);
        this->cameraSystem.OnUpdate(elapsedTime);
        this->cubeCameraSystem.OnUpdate(elapsedTime);
        this->lightSystem.OnUpdate(elapsedTime);
        this->avatarSystem.OnUpdate(elapsedTime);
        this->animationControllerSystem.OnUpdate(elapsedTime);
        this->shadowSystem.OnUpdate(elapsedTime);
        this->particleSystem.OnUpdate(elapsedTime);
        this->postProcessorSystem.OnUpdate(elapsedTime);
    }
    void OnPostUpdate(float elapsedTime)
    {
        this->geometrySystem.OnPostUpdate(elapsedTime);
        this->materialSystem.OnPostUpdate(elapsedTime);
        this->render2DSystem.OnPostUpdate(elapsedTime);
        this->render3DSystem.OnPostUpdate(elapsedTime);
        this->renderSpriteSystem.OnPostUpdate(elapsedTime);
        this->cameraSystem.OnPostUpdate(elapsedTime);
        this->cubeCameraSystem.OnPostUpdate(elapsedTime);
        this->lightSystem.OnPostUpdate(elapsedTime);
        this->avatarSystem.OnPostUpdate(elapsedTime);
        this->animationControllerSystem.OnPostUpdate(elapsedTime);
        this->shadowSystem.OnPostUpdate(elapsedTime);
        this->particleSystem.OnPostUpdate(elapsedTime);
        this->postProcessorSystem.OnPostUpdate(elapsedTime);
    }

    void PostComponentProvider(KG::Component::ComponentProvider& provider)
    {
        this->geometrySystem.OnPostProvider(provider);
        this->materialSystem.OnPostProvider(provider);
        this->render2DSystem.OnPostProvider(provider);
        this->render3DSystem.OnPostProvider(provider);
        this->renderSpriteSystem.OnPostProvider(provider);
        this->cameraSystem.OnPostProvider(provider);
        this->cubeCameraSystem.OnPostProvider(provider);
        this->lightSystem.OnPostProvider(provider);
        this->avatarSystem.OnPostProvider(provider);
        this->animationControllerSystem.OnPostProvider(provider);
        this->shadowSystem.OnPostProvider(provider);
        this->particleSystem.OnPostProvider(provider);
        this->postProcessorSystem.OnPostProvider(provider);
    }

    void Clear()
    {
        this->geometrySystem.Clear();
        this->materialSystem.Clear();
        this->render2DSystem.Clear();
        this->render3DSystem.Clear();
        this->renderSpriteSystem.Clear();
        this->cameraSystem.Clear();
        this->cubeCameraSystem.Clear();
        this->lightSystem.Clear();
        this->avatarSystem.Clear();
        this->animationControllerSystem.Clear();
        this->shadowSystem.Clear();
        this->particleSystem.Clear();
        this->postProcessorSystem.Clear();
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
    TryRelease(dxgiFactory);
    TryRelease(swapChain);
    TryRelease(d3dDevice);
    //TryRelease(rtvDescriptorHeap);
    TryRelease(commandQueue);
    TryRelease(mainCommandAllocator);
    TryRelease(mainCommandList);
    TryRelease(generalRootSignature);
    TryRelease(fence);

    //for (auto*& ptr : this->renderTargetBuffers)
    //{
    //    TryRelease(ptr);
    //}
    DebugNormalMessage("Close D3D12 Renderer");
}

void KGDXRenderer::Initialize()
{
    KGDXRenderer::instance = this;
    DebugNormalMessage("Initilize D3D12 Renderer");
    DebugNormalMessage("RECT : " << this->setting.clientWidth << " , " << this->setting.clientHeight);

    this->CreateD3DDevice();
    this->QueryHardwareFeature();
    this->CreateCommandQueueAndList();
    this->CreateSwapChain();
    this->CreateRtvDescriptorHeaps();
    this->CreateRenderTargetView();
    this->CreateGeneralRootSignature();
    this->CreatePostProcessRootSignature();

    this->renderEngine = std::make_unique<KGRenderEngine>(this->d3dDevice);
    this->graphicSystems = std::make_unique<GraphicSystems>();

    this->CreateSRVDescriptorHeaps();
    this->AllocateGBufferHeap();
    this->InitializeImGui();

    this->particleGenerator.Initialize();
    this->postProcessor = std::make_unique<PostProcessor>();

    this->CreateDXR();
}

void KGDXRenderer::Render()
{

    this->graphicSystems->OnCameraPreRender();
    for (KG::Component::CameraComponent& camera : this->graphicSystems->cameraSystem)
    {
        if (camera.isMainCamera)
        {
            auto frustum = camera.GetFrustum(0.01f, 1000.0f);
            this->graphicSystems->OnProcessCulling(frustum);
        }
    }

    this->graphicSystems->OnPreRender();


    HRESULT hResult = this->mainCommandAllocator->Reset();
    hResult = this->mainCommandList->Reset(this->mainCommandAllocator, nullptr);

    KG::Resource::ResourceContainer::GetInstance()->Process(this->mainCommandList);

    this->mainCommandList->SetGraphicsRootSignature(this->generalRootSignature);
    ID3D12DescriptorHeap* heaps[] = { this->descriptorHeapManager->Get() };
    this->mainCommandList->SetDescriptorHeaps(1, heaps);

    if (this->rtxOn)
    {
        this->dxrScene->UpdateLight(this->graphicSystems->lightSystem.begin(), this->graphicSystems->lightSystem.end());
        this->dxrScene->PrepareRender(dxrDevice, dxrCommandList);
    }

    if (!this->renderEngine->hasRenderJobs())
    {
        this->renderTargetResources[this->swapChainBufferIndex].AddTransitionQueue(D3D12_RESOURCE_STATE_RENDER_TARGET);
        ApplyBarrierQueue(this->mainCommandList);
    }
    if (!this->rtxMain)
    {
        this->ShadowMapRender();
        this->CubeCaemraRender();
        this->NormalCameraRender();
        this->CopyMainCamera();
    }
    else 
    {
        DXRDiffuseRender();
        this->CopyMainCamera();
    }

    //if (this->rtxMain)
    //{
    //    ID3D12Resource* mainCamraData = nullptr;
    //    for (KG::Component::CameraComponent& camera : this->graphicSystems->cameraSystem)
    //    {
    //        if (camera.isMainCamera)
    //        {
    //            mainCamraData = camera.GetCameraDataBuffer();
    //        }
    //    }
    //    //this->DXRRender()
    //}
    //if (this->rtxMain)
    //{
    //    PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Present MainCamera On DXR");
    //    for (KG::Component::CameraComponent& camera : this->graphicSystems->cameraSystem)
    //    {
    //        if (camera.isMainCamera)
    //        {
    //            this->postProcessor->CopyToSwapchainOnDXR(this->mainCommandList, this->dxrRenderer->GetRenderTarget(), this->renderTargetResources[this->swapChainBufferIndex]);
    //        }
    //    }
    //    PIXEndEvent(mainCommandList);
    //}


    this->EditorUIRender();

    hResult = this->mainCommandList->Close();

    ThrowIfFailed(hResult);

    ID3D12CommandList* d3dCommandLists[] = { this->mainCommandList };
    this->commandQueue->ExecuteCommandLists(1, d3dCommandLists);

    this->MoveToNextFrame();


    this->renderEngine->ClearUpdateCount();

    DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
    dxgiPresentParameters.DirtyRectsCount = 0;
    dxgiPresentParameters.pDirtyRects = nullptr;
    dxgiPresentParameters.pScrollRect = nullptr;
    dxgiPresentParameters.pScrollOffset = nullptr;
    this->swapChain->Present1(this->setting.isVsync, 0, &dxgiPresentParameters);

    this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();

    HRESULT reason = this->dxrDevice->GetDeviceRemovedReason();

}

void KG::Renderer::KGDXRenderer::PreRenderEditorUI()
{
    if (!this->isRenderEditUI) return;
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    auto viewportSize = ImGui::GetMainViewport()->Size;
    ImGui::SetNextWindowSize(ImVec2(viewportSize.x, viewportSize.y));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGuiWindowClass a;
    a.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoRendererClear;
    ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0, 0, 0, 0));
    ImGui::SetNextWindowBgAlpha(0);
    ImGui::DockSpaceOverViewport();
}

void KG::Renderer::KGDXRenderer::PreloadModels(std::vector<KG::Utill::HashString>&& ids)
{
    return KG::Resource::ResourceContainer::GetInstance()->PreLoadModels(std::move(ids));
}

void KG::Renderer::KGDXRenderer::CubeCaemraRender()
{
    PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(0), "CubeCameraRender");
    for (KG::Component::CubeCameraComponent& pointLightCamera : this->graphicSystems->cubeCameraSystem)
    {
        for (KG::Component::CameraComponent& cubeCamera : pointLightCamera.GetCameras())
        {
            PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(0), "Cube Camera Render : Camera %d", cubeCamera.GetCubeIndex());

            cubeCamera.SetCameraRender(this->mainCommandList);
            this->OpaqueRender(ShaderGeometryType::Default, ShaderPixelType::Deferred, this->mainCommandList, cubeCamera.GetRenderTexture(), cubeCamera.GetCubeIndex());
            this->TransparentRender(ShaderGeometryType::Default, ShaderPixelType::Transparent, this->mainCommandList, cubeCamera.GetRenderTexture(), cubeCamera.GetCubeIndex());
            this->LightPassRender(this->mainCommandList, cubeCamera.GetRenderTexture(), cubeCamera.GetCubeIndex());
            this->SkyBoxRender(this->mainCommandList, cubeCamera.GetRenderTexture(), cubeCamera.GetCubeIndex());
            this->PassRenderEnd(this->mainCommandList, cubeCamera.GetRenderTexture(), cubeCamera.GetCubeIndex());
            cubeCamera.EndCameraRender(this->mainCommandList);
            PIXEndEvent(mainCommandList);
        }
        pointLightCamera.GetRenderTexture().BarrierTransition(
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
        ApplyBarrierQueue(this->mainCommandList);

    }
    PIXEndEvent(mainCommandList);
}

void KG::Renderer::KGDXRenderer::NormalCameraRender()
{
    PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "NormalCameraRender");
    size_t _cameraCount = 1;
    for (KG::Component::CameraComponent& normalCamera : this->graphicSystems->cameraSystem)
    {
        if(!normalCamera.isMainCamera) continue;
        PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Normal Camera Render : Camera %d", _cameraCount++);
        normalCamera.SetCameraRender(this->mainCommandList);
        this->ParticleReady();
        this->OpaqueRender(ShaderGeometryType::Default, ShaderPixelType::Deferred, this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex(), true);
        this->SSAORender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex(), normalCamera.GetCameraDataBuffer());
        this->LightPassRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->SkyBoxRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->TransparentRender(ShaderGeometryType::Default, ShaderPixelType::Transparent, this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->ParticleRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->SpriteRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->PostProcessRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex(), normalCamera.GetCameraDataBuffer());
        this->InGameUIRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());
        this->PassRenderEnd(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex());

        normalCamera.EndCameraRender(this->mainCommandList);
        PIXEndEvent(mainCommandList);
    }
    PIXEndEvent(mainCommandList);
}

void KG::Renderer::KGDXRenderer::ShadowMapRender()
{
    PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "ShadowCameraRender");
    size_t cameraCount = 0;
    for (KG::Component::ShadowCasterComponent& comp : this->graphicSystems->shadowSystem)
    {
        switch (comp.GetTargetLightType())
        {
        case KG::Component::LightType::DirectionalLight:
        {
            PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Directional Light ShadowMap Render : Camera %d", cameraCount++);
            auto* cascadeCamera = comp.GetDirectionalLightCamera();
            cascadeCamera->SetCameraRender(mainCommandList);
            auto dsvHandle = cascadeCamera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
            this->mainCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            this->mainCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
            this->renderEngine->Render(ShaderGroup::Opaque, ShaderGeometryType::GSCascadeShadow, ShaderPixelType::GSCubeShadow, mainCommandList);
            cascadeCamera->EndCameraRender(mainCommandList);
            PassRenderEnd(mainCommandList, cascadeCamera->GetRenderTexture(), 0);
        }
        break;
        case KG::Component::LightType::PointLight:
        {
            PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Point Light ShadowMap Render : Camera %d", cameraCount++);
            auto* pointLightCamera = comp.GetPointLightCamera();
            pointLightCamera->SetCameraRender(mainCommandList);
            auto dsvHandle = pointLightCamera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
            this->mainCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            this->mainCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
            this->renderEngine->Render(ShaderGroup::Opaque, ShaderGeometryType::GSCubeShadow, ShaderPixelType::GSCubeShadow, mainCommandList);
            pointLightCamera->EndCameraRender(mainCommandList);
            PassRenderEnd(mainCommandList, pointLightCamera->GetRenderTexture(), 0);
        }
        break;
        case KG::Component::LightType::SpotLight:
        {
            PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Directional Light ShadowMap Render : Camera %d", cameraCount++);
            auto* camera = comp.GetSpotLightCamera();
            camera->SetCameraRender(mainCommandList);
            auto dsvHandle = camera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
            this->mainCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
            this->mainCommandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
            this->renderEngine->Render(ShaderGroup::Opaque, ShaderGeometryType::Default, ShaderPixelType::Shadow, mainCommandList);
            camera->EndCameraRender(mainCommandList);
            PassRenderEnd(mainCommandList, camera->GetRenderTexture(), 0);
        }
        break;
        }
        PIXEndEvent(mainCommandList);
    }
    PIXEndEvent(mainCommandList);
}

void KG::Renderer::KGDXRenderer::EditorUIRender()
{
    if (this->isRenderEditUI)
    {
        this->renderTargetResources[this->swapChainBufferIndex].AddTransitionQueue(D3D12_RESOURCE_STATE_RENDER_TARGET);
        ApplyBarrierQueue(this->mainCommandList);
        ImGui::PopStyleColor(1);
        PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(0), "ImGui UI Render");
        auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart())
            .Offset(this->swapChainBufferIndex, this->hwFeature.rtvDescriptorSize);
        this->mainCommandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), this->mainCommandList);
        ImGui::EndFrame();
    }
    PIXEndEvent(mainCommandList);
    this->renderTargetResources[this->swapChainBufferIndex].AddTransitionQueue(D3D12_RESOURCE_STATE_PRESENT);
    ApplyBarrierQueue(this->mainCommandList);
}

void KG::Renderer::KGDXRenderer::CopyMainCamera()
{
    this->mainCommandList->SetComputeRootSignature(this->postProcessRootSignature);
    PIXBeginEvent(mainCommandList, PIX_COLOR_INDEX(1), "Present MainCamera");
    for (KG::Component::CameraComponent& camera : this->graphicSystems->cameraSystem)
    {
        if (camera.isMainCamera)
        {
            this->postProcessor->CopyToSwapchain(this->mainCommandList, camera.GetRenderTexture().renderTargetResource, this->renderTargetResources[this->swapChainBufferIndex]);
        }
    }
    PIXEndEvent(mainCommandList);
}

void KG::Renderer::KGDXRenderer::OpaqueRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, bool culled)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "Opaque Render");
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 4> gbufferHandle =
    {
        rt.gbufferTextureResources[0].GetDescriptor(DescriptorType::RTV).GetCPUHandle(),
        rt.gbufferTextureResources[1].GetDescriptor(DescriptorType::RTV).GetCPUHandle(),
        rt.gbufferTextureResources[2].GetDescriptor(DescriptorType::RTV).GetCPUHandle(),
        rt.gbufferTextureResources[3].GetDescriptor(DescriptorType::RTV).GetCPUHandle()
    };
    auto dsvHandle = rt.depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
    cmdList->OMSetRenderTargets(4, gbufferHandle.data(), false, &dsvHandle);
    rt.ClearGBuffer(this->mainCommandList, 0, 0, 0, 0);
    cmdList->SetGraphicsRootDescriptorTable(GraphicRootParameterIndex::Texture, this->descriptorHeapManager->GetGPUHandle(0));
    cmdList->SetGraphicsRootDescriptorTable(GraphicRootParameterIndex::TextureArray, this->descriptorHeapManager->GetGPUHandle(0));
    cmdList->SetGraphicsRootDescriptorTable(GraphicRootParameterIndex::TextureCube, this->descriptorHeapManager->GetGPUHandle(0));

    this->mainCommandList->ClearDepthStencilView(dsvHandle,
        D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr);
    this->renderEngine->Render(ShaderGroup::Opaque, geoType, pixType, cmdList, culled);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::TransparentRender(ShaderGeometryType geoType, ShaderPixelType pixType, ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "Transparent Render");
    this->renderEngine->Render(ShaderGroup::Transparent, geoType, pixType, cmdList);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::ParticleReady()
{
    this->particleGenerator.PreRenderStart();
}

void KG::Renderer::KGDXRenderer::DXRDiffuseRender()
{
    this->dxrCommandList->SetComputeRootSignature(this->dxrRootSignature);

    for (KG::Component::CameraComponent& normalCamera : this->graphicSystems->cameraSystem)
    {
        if (!normalCamera.isMainCamera) continue;
        normalCamera.SetCameraRender(dxrCommandList);
        dxrCommandList->SetComputeRootConstantBufferView(DXRRootParameterIndex::CameraData, normalCamera.GetCameraDataBuffer()->GetGPUVirtualAddress());
        dxrCommandList->SetComputeRootDescriptorTable(DXRRootParameterIndex::Texture, this->descriptorHeapManager->GetGPUHandle(0));
        dxrCommandList->SetComputeRootDescriptorTable(DXRRootParameterIndex::TextureArray, this->descriptorHeapManager->GetGPUHandle(0));
        dxrCommandList->SetComputeRootDescriptorTable(DXRRootParameterIndex::TextureCube, this->descriptorHeapManager->GetGPUHandle(0));
        this->dxrScene->Render(normalCamera.GetRenderTexture().renderTargetResource, dxrCommandList, this->setting.clientWidth, this->setting.clientHeight);
        this->PostProcessRender(this->mainCommandList, normalCamera.GetRenderTexture(), normalCamera.GetCubeIndex(), normalCamera.GetCameraDataBuffer());
        normalCamera.EndCameraRender(dxrCommandList);
        break;
    }
}

void KG::Renderer::KGDXRenderer::ParticleRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "Particle Render");
    this->particleGenerator.PreRenderCheck();
    this->renderEngine->Render(ShaderGroup::ParticleTransparent, ShaderGeometryType::Particle, ShaderPixelType::Transparent, cmdList);
    this->renderEngine->Render(ShaderGroup::ParticleAdd, ShaderGeometryType::Particle, ShaderPixelType::Add, cmdList);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::SpriteRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "Sprite Render");
    this->renderEngine->Render(ShaderGroup::Sprite, ShaderGeometryType::Particle, ShaderPixelType::Transparent, cmdList);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::InGameUIRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "InGameUI Render");
    rt.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );
    ApplyBarrierQueue(cmdList);
    auto dsvHandle = rt.depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    this->renderEngine->Render(ShaderGroup::UI, ShaderGeometryType::Particle, ShaderPixelType::Transparent, cmdList);
    rt.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    ApplyBarrierQueue(cmdList);
    PIXEndEvent(cmdList);
}



void KG::Renderer::KGDXRenderer::SSAORender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, ID3D12Resource* data)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "SSAO Pass Render");
    rt.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    this->postProcessor->SSAO(cmdList, rt, cubeIndex, data);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::LightPassRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "Light Pass Render");
    rt.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    ApplyBarrierQueue(cmdList);
    auto gbufferTable = rt.gbufferTextureResources[0].GetDescriptor(DescriptorType::SRV).GetGPUHandle();
    cmdList->SetGraphicsRootDescriptorTable(GraphicRootParameterIndex::GBufferHeap, gbufferTable);
    auto rtvHandle = rt.GetRenderTargetRTVHandle(cubeIndex);
    cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);
    this->renderEngine->Render(ShaderGroup::DirectionalLight, ShaderGeometryType::Light, ShaderPixelType::Light, cmdList);
    this->renderEngine->Render(ShaderGroup::MeshVolumeLight, ShaderGeometryType::Light, ShaderPixelType::Light, ShaderTesselation::LightVolumeMesh, cmdList);
    this->renderEngine->Render(ShaderGroup::AmbientLight, ShaderGeometryType::Light, ShaderPixelType::Light, cmdList);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::SkyBoxRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "SkyBox Pass Render");
    auto rtvHandle = rt.GetRenderTargetRTVHandle(cubeIndex);
    auto dsvHandle = rt.depthStencilBuffer.GetDescriptor(DescriptorType::DSV).GetCPUHandle();
    cmdList->OMSetRenderTargets(1, &rtvHandle, true,&dsvHandle);
    this->renderEngine->Render(ShaderGroup::SkyBox, ShaderGeometryType::SkyBox, ShaderPixelType::SkyBox, cmdList);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::PostProcessRender(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex, ID3D12Resource* data)
{
    PIXBeginEvent(cmdList, PIX_COLOR_INDEX(2), "PostProcess Pass Render");
    this->postProcessor->Draw(cmdList, rt, cubeIndex, data);
    PIXEndEvent(cmdList);
}

void KG::Renderer::KGDXRenderer::PassRenderEnd(ID3D12GraphicsCommandList* cmdList, KG::Renderer::RenderTexture& rt, size_t cubeIndex)
{
    rt.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_WRITE
    );
    ApplyBarrierQueue(cmdList);
}

KG::Renderer::ParticleGenerator* KG::Renderer::KGDXRenderer::GetParticleGenerator()
{
    return &this->particleGenerator;
}

void KG::Renderer::KGDXRenderer::Update(float elapsedTime)
{
    this->graphicSystems->OnUpdate(elapsedTime);
    this->graphicSystems->OnPostUpdate(elapsedTime);
    this->particleGenerator.Update(elapsedTime);
}

void KGDXRenderer::OnChangeSettings(const RendererSetting& prev, const RendererSetting& next)
{
    NotImplement(KGDXRenderer::OnChangeSettings);
}

void KG::Renderer::KGDXRenderer::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
    this->graphicSystems->PostComponentProvider(provider);
}

void* KG::Renderer::KGDXRenderer::GetImGUIContext()
{
    return ImGui::GetCurrentContext();
}

KG::Component::IRender3DComponent* KG::Renderer::KGDXRenderer::GetNewRenderComponent()
{
    return static_cast<KG::Component::IRender3DComponent*>(this->graphicSystems->render3DSystem.GetNewComponent());
}

KG::Component::IRender2DComponent* KG::Renderer::KGDXRenderer::GetNewRender2DComponent()
{
    return static_cast<KG::Component::IRender2DComponent*>(this->graphicSystems->render2DSystem.GetNewComponent());
}

KG::Component::IRenderSpriteComponent* KG::Renderer::KGDXRenderer::GetNewRenderSpriteComponent()
{
    return static_cast<KG::Component::IRenderSpriteComponent*>(this->graphicSystems->renderSpriteSystem.GetNewComponent());
}

KG::Component::IGeometryComponent* KG::Renderer::KGDXRenderer::GetNewGeomteryComponent()
{
    return static_cast<KG::Component::IGeometryComponent*>(this->graphicSystems->geometrySystem.GetNewComponent());
}

KG::Component::IMaterialComponent* KG::Renderer::KGDXRenderer::GetNewMaterialComponent()
{
    return static_cast<KG::Component::IMaterialComponent*>(this->graphicSystems->materialSystem.GetNewComponent());
}

KG::Component::ICameraComponent* KG::Renderer::KGDXRenderer::GetNewCameraComponent()
{
    return static_cast<KG::Component::ICameraComponent*>(this->graphicSystems->cameraSystem.GetNewComponent());
}

KG::Component::ICubeCameraComponent* KG::Renderer::KGDXRenderer::GetNewCubeCameraComponent()
{
    return static_cast<KG::Component::ICubeCameraComponent*>(this->graphicSystems->cubeCameraSystem.GetNewComponent());
}

void KG::Renderer::KGDXRenderer::DebugUIRender()
{
    float sizeX = (ImGui::GetColumnWidth()) - 10.0f;
    ImGui::Checkbox("DXR ON", &this->rtxOn);
    ImGui::Checkbox("DXR MAIN", &this->rtxMain);
    //if (ImGui::TreeNode("DXR"))
    //{
    //    auto ptr = this->dxrRenderer->GetRenderTarget().GetDescriptor(DescriptorType::SRV).GetGPUHandle().ptr;
    //    ImGui::TextureView((ImTextureID)ptr, ImVec2(sizeX, sizeX * (9.0f / 16.0f)), "DXR Result");
    //    ImGui::TreePop();
    //}
    if (ImGui::TreeNode("RenderTexture : deferred render"))
    {
        for (auto& camera : this->graphicSystems->cameraSystem)
        {
            if (camera.isMainCamera)
            {
                static const char* strings[] = {
                    "gbuffer0\ndiffuse color(rgb)",
                    "gbuffer1\nspecular (r), metalic (g), roughness (b)",
                    "gbuffer2\nnormal (xy)",
                    "gbuffer3\nenvirionment map index (x)",
                    "gbuffer4\ndepth buffer (x)",
                };
                for (size_t i = 0; i < 4; i++)
                {
                    auto ptr = camera.GetRenderTexture().gbufferTextureResources[i].GetDescriptor(DescriptorType::SRV).GetGPUHandle().ptr;
                    //ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text(strings[i]);
                    ImGui::TextureView((ImTextureID)ptr, ImVec2(sizeX, sizeX * (9.0f / 16.0f)), strings[i]);
                    ImGui::EndGroup();
                }
                {
                    auto ptr = camera.GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV).GetGPUHandle().ptr;
                    //ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text(strings[4]);
                    ImGui::TextureView((ImTextureID)ptr, ImVec2(sizeX, sizeX * (9.0f / 16.0f)), strings[4]);
                    ImGui::EndGroup();
                }
                break;
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("PostProcess"))
    {
        this->postProcessor->OnDrawGUI();
        ImGui::TreePop();
    }
    static KG::Component::CameraComponent* mainCamera = nullptr;
    static KG::Component::CameraComponent* sceneCamera = nullptr;
    if (ImGui::Button("ToggleCamera"))
    {
        if (mainCamera == nullptr)
        {
            for (auto& camera : this->graphicSystems->cameraSystem)
            {
                if (camera.GetGameObject()->tag == "SceneCameraObject"_id)
                    sceneCamera = &camera;
                if (camera.GetGameObject()->tag == "FPCamera"_id)
                    mainCamera = &camera;
            }
        }
        if (mainCamera->isMainCamera)
        {
            sceneCamera->SetMainCamera();
        }
        else 
        {
            mainCamera->SetMainCamera();
        }
    }
}

KG::Component::ILightComponent* KG::Renderer::KGDXRenderer::GetNewLightComponent()
{
    return static_cast<KG::Component::ILightComponent*>(this->graphicSystems->lightSystem.GetNewComponent());
}

KG::Component::IShadowCasterComponent* KG::Renderer::KGDXRenderer::GetNewShadowCasterComponent()
{
    return static_cast<KG::Component::IShadowCasterComponent*>(this->graphicSystems->shadowSystem.GetNewComponent());
}

KG::Component::IBoneTransformComponent* KG::Renderer::KGDXRenderer::GetNewBoneTransformComponent()
{
    return static_cast<KG::Component::IBoneTransformComponent*>(this->graphicSystems->avatarSystem.GetNewComponent());
}

KG::Component::IAnimationControllerComponent* KG::Renderer::KGDXRenderer::GetNewAnimationControllerComponent()
{
    auto* anim = static_cast<KG::Component::IAnimationControllerComponent*>(this->graphicSystems->animationControllerSystem.GetNewComponent());
    return anim;
}

KG::Component::IParticleEmitterComponent* KG::Renderer::KGDXRenderer::GetNewParticleEmitterComponent()
{
    auto* particleComp = static_cast<KG::Component::IParticleEmitterComponent*>(this->graphicSystems->particleSystem.GetNewComponent());
    return particleComp;
}

KG::Component::IPostProcessManagerComponent* KG::Renderer::KGDXRenderer::GetNewPostProcessorComponent()
{
    auto* postComp = static_cast<KG::Component::IPostProcessManagerComponent*>(this->graphicSystems->postProcessorSystem.GetNewComponent());
    return postComp;
}

KG::Core::GameObject* KG::Renderer::KGDXRenderer::LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials)
{
    return KG::Resource::ResourceContainer::GetInstance()->CreateObjectFromModel(id, container, materials);
}

KG::Core::GameObject* KG::Renderer::KGDXRenderer::LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials)
{
    return KG::Resource::ResourceContainer::GetInstance()->CreateObjectFromModel(id, scene, materials);
}

RendererSetting KG::Renderer::KGDXRenderer::GetSetting() const
{
    return this->setting;
}

ID3D12Device* KG::Renderer::KGDXRenderer::GetD3DDevice() const
{
    return this->d3dDevice;
}

UINT64 KG::Renderer::KGDXRenderer::GetFenceValue() const
{
    return this->fenceValue;
}

KG::Renderer::PostProcessor* KG::Renderer::KGDXRenderer::GetPostProcess() const
{
    return this->postProcessor.get();
}

bool KG::Renderer::KGDXRenderer::isRayRender() const
{
    return rtxOn || rtxMain;
}

KG::Renderer::KGDXRenderer* KG::Renderer::KGDXRenderer::GetInstance()
{
    return KGDXRenderer::instance;
}

ID3D12RootSignature* KG::Renderer::KGDXRenderer::GetGeneralRootSignature() const
{
    return this->generalRootSignature;
}

ID3D12RootSignature* KG::Renderer::KGDXRenderer::GetPostProcessRootSignature() const
{
    return this->postProcessRootSignature;
}

KGRenderEngine* KG::Renderer::KGDXRenderer::GetRenderEngine() const
{
    return this->renderEngine.get();
}

const HardwareFeature& KG::Renderer::KGDXRenderer::GetHWFeature() const
{
    return this->hwFeature;
}

DescriptorHeapManager* KG::Renderer::KGDXRenderer::GetDescriptorHeapManager() const
{
    return this->descriptorHeapManager.get();
}

KG::Renderer::DXRRenderScene* KG::Renderer::KGDXRenderer::GetDXRScene() const
{
    return this->dxrScene;
}

void KG::Renderer::KGDXRenderer::AddNewRenderJob(KGRenderJob* job)
{
    if (this->hwFeature.isAbleDXR)
    {
        this->dxrScene->AddNewRenderJobs(this->dxrDevice, job);
    }
}

double KG::Renderer::KGDXRenderer::GetGameTime() const
{
    return this->gameTime;
}

UINT KG::Renderer::KGDXRenderer::QueryMaterialIndex(const KG::Utill::HashString& materialId) const
{
    return KG::Resource::ResourceContainer::GetInstance()->LoadMaterial(materialId).first;
}

void KG::Renderer::KGDXRenderer::SetEditUIRender(bool isRender)
{
    this->isRenderEditUI = isRender;
}

void KG::Renderer::KGDXRenderer::SetGameTime(double gameTime)
{
    this->gameTime = gameTime;
}

void KG::Renderer::KGDXRenderer::QueryHardwareFeature()
{
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureSupport;
    ZeroDesc(featureSupport);

    this->hwFeature.srvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    this->hwFeature.dsvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    this->hwFeature.rtvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    this->hwFeature.frameResourceCount = 3;

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {}; 
    ThrowIfFailed(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
    if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0)
    {
        DebugNormalMessage("This System Can't Use DXR");
    }
    else
    {
        this->hwFeature.isAbleDXR = true;
    }
}

void KG::Renderer::KGDXRenderer::CreateDXR()
{
    this->dxrScene = new DXRRenderScene();
    if (this->hwFeature.isAbleDXR)
    {
        this->d3dDevice->QueryInterface(&this->dxrDevice);
        this->mainCommandList->QueryInterface(&this->dxrCommandList);
    }
    this->CreateDXRRootSignature();
    this->dxrScene->Initialize(this->dxrDevice, this->dxrRootSignature);
}

void KG::Renderer::KGDXRenderer::CreateD3DDevice()
{
    HRESULT hResult;
    UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) | defined(DEBUF)
    ID3D12Debug3* d3dDebugController = nullptr;
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
            DebugNormalMessage("Init as Adapter : " << dxgiAdapterDesc.Description);
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

    this->setting.msaa4xQualityLevel = 999;
    this->setting.msaa4xEnable = true;

    this->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(decltype(d3dMsaaQualityLevels)));
    this->setting.msaa4xQualityLevel = std::min(this->setting.msaa4xQualityLevel, d3dMsaaQualityLevels.NumQualityLevels);
    this->setting.msaa4xEnable = this->setting.msaa4xEnable & (this->setting.msaa4xQualityLevel > 1);

    hResult = this->d3dDevice->CreateFence(0, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence));
    this->hFenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
    this->fenceValue = 0;
    TryRelease(d3dAdapter);
    ;
}

void KG::Renderer::KGDXRenderer::CreateSwapChain()
{
    //RECT clientRect;
    //::GetClientRect(this->desc.hWnd, &clientRect);
    //this->setting.clientWidth = clientRect.right - clientRect.left;
    //this->setting.clientHeight = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
    ::ZeroMemory(&dxgiSwapChainDesc, sizeof(decltype(dxgiSwapChainDesc)));
    dxgiSwapChainDesc.Width = this->setting.clientWidth;
    dxgiSwapChainDesc.Height = this->setting.clientHeight;
    //dxgiSwapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.SampleDesc.Count = (this->setting.msaa4xEnable) ? 4 : 1;
    dxgiSwapChainDesc.SampleDesc.Quality = (this->setting.msaa4xEnable) ? (this->setting.msaa4xQualityLevel - 1) : 0;
    dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.BufferCount = this->setting.maxSwapChainCount;
    dxgiSwapChainDesc.Scaling = DXGI_SCALING::DXGI_SCALING_NONE;
    dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
    //dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE::DXGI_ALPHA_MODE_UNSPECIFIED;
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

    this->rtvDescriptorHeap.Initialize(d3dDevice, d3dDescriptorHeapDesc, this->hwFeature.rtvDescriptorSize);
}

void KG::Renderer::KGDXRenderer::CreateSRVDescriptorHeaps()
{
    this->descriptorHeapManager = std::make_unique<DescriptorHeapManager>();
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    // ! 서술자 갯수
    srvHeapDesc.NumDescriptors = 120000;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    this->descriptorHeapManager->Initialize(this->d3dDevice, srvHeapDesc, this->hwFeature.srvDescriptorSize);
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
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    ZeroDesc(rtvDesc);
    rtvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;
    this->renderTargetResources.resize(this->setting.maxSwapChainCount);
    for (size_t i = 0; i < this->setting.maxSwapChainCount; i++)
    {
        ID3D12Resource* resource = nullptr;
        this->swapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
        this->renderTargetResources[i].SetResource(resource);
        this->renderTargetResources[i].AddOnDescriptorHeap(&this->rtvDescriptorHeap, rtvDesc);
    }
}

void KG::Renderer::KGDXRenderer::InitializeImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    this->imguiFontDescIndex = this->descriptorHeapManager->RequestEmptyIndex();
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(this->desc.hWnd);
    ImGui_ImplDX12_Init(this->d3dDevice, this->renderTargetResources.size(),
        DXGI_FORMAT_R8G8B8A8_UNORM, this->descriptorHeapManager->Get(),
        this->descriptorHeapManager->GetCPUHandle(this->imguiFontDescIndex),
        this->descriptorHeapManager->GetGPUHandle(this->imguiFontDescIndex));
}

void KG::Renderer::KGDXRenderer::CreateGeneralRootSignature()
{
    D3D12_ROOT_PARAMETER pd3dRootParameters[10]{};

    // 0 : Space 0 : SRV 0 : Instance Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::InstanceData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[GraphicRootParameterIndex::InstanceData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[GraphicRootParameterIndex::InstanceData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[GraphicRootParameterIndex::InstanceData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 1 : Space 3 : SRV 1 : Animation Transform Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::AnimationTransformData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[GraphicRootParameterIndex::AnimationTransformData].Descriptor.ShaderRegister = 1;
    pd3dRootParameters[GraphicRootParameterIndex::AnimationTransformData].Descriptor.RegisterSpace = 3;
    pd3dRootParameters[GraphicRootParameterIndex::AnimationTransformData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 2 : Space 3 : SRV 0 : Bone Offset Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::BoneOffsetData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[GraphicRootParameterIndex::BoneOffsetData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[GraphicRootParameterIndex::BoneOffsetData].Descriptor.RegisterSpace = 3;
    pd3dRootParameters[GraphicRootParameterIndex::BoneOffsetData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 3 : Space 0 : SRV 1 : Material Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::MaterialData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[GraphicRootParameterIndex::MaterialData].Descriptor.ShaderRegister = 1;
    pd3dRootParameters[GraphicRootParameterIndex::MaterialData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[GraphicRootParameterIndex::MaterialData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 4 : Space 0 : CBV 0 : Camera Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::CameraData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[GraphicRootParameterIndex::CameraData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[GraphicRootParameterIndex::CameraData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[GraphicRootParameterIndex::CameraData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 5 : Space 4 : CBV 1 : Pass Data : 2
    pd3dRootParameters[GraphicRootParameterIndex::LightData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[GraphicRootParameterIndex::LightData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[GraphicRootParameterIndex::LightData].Descriptor.RegisterSpace = 4;
    pd3dRootParameters[GraphicRootParameterIndex::LightData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 6 : Space 1 : SRV 0 : Texture Data1 // unbounded : 1

    D3D12_DESCRIPTOR_RANGE txtureData1Range;
    ZeroDesc(txtureData1Range);
    txtureData1Range.BaseShaderRegister = 0;
    txtureData1Range.NumDescriptors = -1;
    txtureData1Range.OffsetInDescriptorsFromTableStart = 0;
    txtureData1Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureData1Range.RegisterSpace = 1;

    pd3dRootParameters[GraphicRootParameterIndex::Texture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[GraphicRootParameterIndex::Texture].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[GraphicRootParameterIndex::Texture].DescriptorTable.pDescriptorRanges = &txtureData1Range;
    pd3dRootParameters[GraphicRootParameterIndex::Texture].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    // 7 : Space 2 : SRV 0 : Texture Array // unbounded : 1

    D3D12_DESCRIPTOR_RANGE txtureData2Range;
    ZeroDesc(txtureData2Range);
    txtureData2Range.BaseShaderRegister = 0;
    txtureData2Range.NumDescriptors = -1;
    txtureData2Range.OffsetInDescriptorsFromTableStart = 0;
    txtureData2Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureData2Range.RegisterSpace = 5;

    pd3dRootParameters[GraphicRootParameterIndex::TextureArray].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[GraphicRootParameterIndex::TextureArray].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[GraphicRootParameterIndex::TextureArray].DescriptorTable.pDescriptorRanges = &txtureData2Range;
    pd3dRootParameters[GraphicRootParameterIndex::TextureArray].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    // 8 : space 3 : SRV 0 : Texture Cube // unbounded : 1
    D3D12_DESCRIPTOR_RANGE txtureCubeRange;
    ZeroDesc(txtureCubeRange);
    txtureCubeRange.BaseShaderRegister = 0;
    txtureCubeRange.NumDescriptors = -1;
    txtureCubeRange.OffsetInDescriptorsFromTableStart = 0;
    txtureCubeRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureCubeRange.RegisterSpace = 2;

    pd3dRootParameters[GraphicRootParameterIndex::TextureCube].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[GraphicRootParameterIndex::TextureCube].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[GraphicRootParameterIndex::TextureCube].DescriptorTable.pDescriptorRanges = &txtureCubeRange;
    pd3dRootParameters[GraphicRootParameterIndex::TextureCube].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    // 9 : Space 0 : SRV 2,3,4,5,6 : G Buffer // unbounded : 1

    D3D12_DESCRIPTOR_RANGE GbufferRange;
    ZeroDesc(GbufferRange);
    GbufferRange.BaseShaderRegister = 2;
    GbufferRange.NumDescriptors = 5;
    GbufferRange.OffsetInDescriptorsFromTableStart = 0;
    GbufferRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    GbufferRange.RegisterSpace = 0;

    pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].DescriptorTable.pDescriptorRanges = &GbufferRange;
    pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    //D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
    //	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    auto staticSampler = GetStaticSamplers();

    ZeroDesc(d3dRootSignatureDesc);
    d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
    d3dRootSignatureDesc.pParameters = pd3dRootParameters;
    d3dRootSignatureDesc.NumStaticSamplers = staticSampler.size();
    d3dRootSignatureDesc.pStaticSamplers = staticSampler.data();
    d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

    ID3DBlob* pd3dSignatureBlob = nullptr;
    ID3DBlob* pd3dErrorBlob = nullptr;

    auto result = ::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
    if (FAILED(result))
    {
        DebugErrorMessage((const char*)pd3dErrorBlob->GetBufferPointer());
    }
    this->d3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&this->generalRootSignature));
    TryRelease(pd3dSignatureBlob);
    TryRelease(pd3dErrorBlob);
}

void KG::Renderer::KGDXRenderer::CreateDXRRootSignature()
{
    D3D12_ROOT_PARAMETER pd3dRootParameters[9]{};

    pd3dRootParameters[DXRRootParameterIndex::InstanceIndexData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[DXRRootParameterIndex::InstanceIndexData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[DXRRootParameterIndex::InstanceIndexData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[DXRRootParameterIndex::InstanceIndexData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[DXRRootParameterIndex::LightData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[DXRRootParameterIndex::LightData].Descriptor.ShaderRegister = 1;
    pd3dRootParameters[DXRRootParameterIndex::LightData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[DXRRootParameterIndex::LightData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[DXRRootParameterIndex::CameraData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[DXRRootParameterIndex::CameraData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[DXRRootParameterIndex::CameraData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[DXRRootParameterIndex::CameraData].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[DXRRootParameterIndex::Ambient].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[DXRRootParameterIndex::Ambient].Descriptor.ShaderRegister = 1;
    pd3dRootParameters[DXRRootParameterIndex::Ambient].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[DXRRootParameterIndex::Ambient].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    D3D12_DESCRIPTOR_RANGE UAVRange;
    ZeroDesc(UAVRange);
    UAVRange.BaseShaderRegister = 0;
    UAVRange.NumDescriptors = 1;
    UAVRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    UAVRange.RegisterSpace = 0;
    UAVRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

    pd3dRootParameters[DXRRootParameterIndex::RWTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[DXRRootParameterIndex::RWTexture].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[DXRRootParameterIndex::RWTexture].DescriptorTable.pDescriptorRanges = &UAVRange;
    pd3dRootParameters[DXRRootParameterIndex::RWTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[DXRRootParameterIndex::AccelerationStructure].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[DXRRootParameterIndex::AccelerationStructure].Descriptor.ShaderRegister = 2;
    pd3dRootParameters[DXRRootParameterIndex::AccelerationStructure].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[DXRRootParameterIndex::AccelerationStructure].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;


    // 6 : Space 1 : SRV 0 : Texture Data1 // unbounded : 1

    D3D12_DESCRIPTOR_RANGE txtureData1Range;
    ZeroDesc(txtureData1Range);
    txtureData1Range.BaseShaderRegister = 0;
    txtureData1Range.NumDescriptors = -1;
    txtureData1Range.OffsetInDescriptorsFromTableStart = 0;
    txtureData1Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureData1Range.RegisterSpace = 1;

    pd3dRootParameters[DXRRootParameterIndex::Texture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[DXRRootParameterIndex::Texture].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[DXRRootParameterIndex::Texture].DescriptorTable.pDescriptorRanges = &txtureData1Range;
    pd3dRootParameters[DXRRootParameterIndex::Texture].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 7 : Space 2 : SRV 0 : Texture Array // unbounded : 1

    D3D12_DESCRIPTOR_RANGE txtureData2Range;
    ZeroDesc(txtureData2Range);
    txtureData2Range.BaseShaderRegister = 0;
    txtureData2Range.NumDescriptors = -1;
    txtureData2Range.OffsetInDescriptorsFromTableStart = 0;
    txtureData2Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureData2Range.RegisterSpace = 2;

    pd3dRootParameters[DXRRootParameterIndex::TextureArray].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[DXRRootParameterIndex::TextureArray].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[DXRRootParameterIndex::TextureArray].DescriptorTable.pDescriptorRanges = &txtureData2Range;
    pd3dRootParameters[DXRRootParameterIndex::TextureArray].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 8 : space 3 : SRV 0 : Texture Cube // unbounded : 1
    D3D12_DESCRIPTOR_RANGE txtureCubeRange;
    ZeroDesc(txtureCubeRange);
    txtureCubeRange.BaseShaderRegister = 0;
    txtureCubeRange.NumDescriptors = -1;
    txtureCubeRange.OffsetInDescriptorsFromTableStart = 0;
    txtureCubeRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    txtureCubeRange.RegisterSpace = 3;

    pd3dRootParameters[DXRRootParameterIndex::TextureCube].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[DXRRootParameterIndex::TextureCube].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[DXRRootParameterIndex::TextureCube].DescriptorTable.pDescriptorRanges = &txtureCubeRange;
    pd3dRootParameters[DXRRootParameterIndex::TextureCube].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;

    // 9 : Space 0 : SRV 2,3,4,5,6 : G Buffer // unbounded : 1

    //D3D12_DESCRIPTOR_RANGE GbufferRange;
    //ZeroDesc(GbufferRange);
    //GbufferRange.BaseShaderRegister = 2;
    //GbufferRange.NumDescriptors = 5;
    //GbufferRange.OffsetInDescriptorsFromTableStart = 0;
    //GbufferRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    //GbufferRange.RegisterSpace = 0;

    //pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    //pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].DescriptorTable.NumDescriptorRanges = 1;
    //pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].DescriptorTable.pDescriptorRanges = &GbufferRange;
    //pd3dRootParameters[GraphicRootParameterIndex::GBufferHeap].ShaderVisibility = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    //D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
    //	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    auto staticSampler = GetStaticSamplers();

    ZeroDesc(d3dRootSignatureDesc);
    d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
    d3dRootSignatureDesc.pParameters = pd3dRootParameters;
    d3dRootSignatureDesc.NumStaticSamplers = staticSampler.size();
    d3dRootSignatureDesc.pStaticSamplers = staticSampler.data();
    d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

    ID3DBlob* pd3dSignatureBlob = nullptr;
    ID3DBlob* pd3dErrorBlob = nullptr;

    auto result = ::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
    if (FAILED(result))
    {
        DebugErrorMessage((const char*)pd3dErrorBlob->GetBufferPointer());
    }
    this->d3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&this->dxrRootSignature));
    TryRelease(pd3dSignatureBlob);
    TryRelease(pd3dErrorBlob);
}

void KG::Renderer::KGDXRenderer::CreatePostProcessRootSignature()
{
    //딱히 디스크립터 힙으로 엮으라는 말 없다! -> 텍스처는 디스크립터힙만 된다!
    D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[7];

    pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    pd3dDescriptorRanges[0].NumDescriptors = 1;
    pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Result
    pd3dDescriptorRanges[0].RegisterSpace = 0;
    pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[1].NumDescriptors = 1;
    pd3dDescriptorRanges[1].BaseShaderRegister = 0; // 이전 결과
    pd3dDescriptorRanges[1].RegisterSpace = 0;
    pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[2].NumDescriptors = 1;
    pd3dDescriptorRanges[2].BaseShaderRegister = 0; // 후처리 미적용 최종 결과
    pd3dDescriptorRanges[2].RegisterSpace = 1;
    pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[3].NumDescriptors = 5; // GBuffer 1, 2, 3, 4 / Depth Stencil
    pd3dDescriptorRanges[3].BaseShaderRegister = 1; // GBuffer
    pd3dDescriptorRanges[3].RegisterSpace = 1;
    pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    pd3dDescriptorRanges[4].NumDescriptors = 3;
    pd3dDescriptorRanges[4].BaseShaderRegister = 0; //Result
    pd3dDescriptorRanges[4].RegisterSpace = 2;
    pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    pd3dDescriptorRanges[5].NumDescriptors = 3;
    pd3dDescriptorRanges[5].BaseShaderRegister = 0; //Result
    pd3dDescriptorRanges[5].RegisterSpace = 3;
    pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    pd3dDescriptorRanges[6].NumDescriptors = 1;
    pd3dDescriptorRanges[6].BaseShaderRegister = 0; //Result
    pd3dDescriptorRanges[6].RegisterSpace = 3;
    pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER pd3dRootParameters[10];
    ZeroMemory(pd3dRootParameters, 10 * sizeof(D3D12_ROOT_PARAMETER));

    pd3dRootParameters[ComputeRootParameterIndex::Result].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::Result].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::Result].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[0]);
    pd3dRootParameters[ComputeRootParameterIndex::Result].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::PrevResult].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::PrevResult].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::PrevResult].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[1]);
    pd3dRootParameters[ComputeRootParameterIndex::PrevResult].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::Source].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::Source].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::Source].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[2]);
    pd3dRootParameters[ComputeRootParameterIndex::Source].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::GBufferStart].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::GBufferStart].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::GBufferStart].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[3]);
    pd3dRootParameters[ComputeRootParameterIndex::GBufferStart].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::UAVBuffers].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::UAVBuffers].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::UAVBuffers].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[4]);
    pd3dRootParameters[ComputeRootParameterIndex::UAVBuffers].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::SRVBuffers].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::SRVBuffers].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::SRVBuffers].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[5]);
    pd3dRootParameters[ComputeRootParameterIndex::SRVBuffers].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::MaterialData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialData].Descriptor.RegisterSpace = 2;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialData].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::MaterialIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialIndex].Constants.Num32BitValues = 1;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialIndex].Constants.RegisterSpace = 1;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialIndex].Constants.ShaderRegister = 0;
    pd3dRootParameters[ComputeRootParameterIndex::MaterialIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::CameraData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    pd3dRootParameters[ComputeRootParameterIndex::CameraData].Descriptor.RegisterSpace = 0;
    pd3dRootParameters[ComputeRootParameterIndex::CameraData].Descriptor.ShaderRegister = 0;
    pd3dRootParameters[ComputeRootParameterIndex::CameraData].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    pd3dRootParameters[ComputeRootParameterIndex::FrameData].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    pd3dRootParameters[ComputeRootParameterIndex::FrameData].DescriptorTable.NumDescriptorRanges = 1;
    pd3dRootParameters[ComputeRootParameterIndex::FrameData].DescriptorTable.pDescriptorRanges = &(pd3dDescriptorRanges[6]);
    pd3dRootParameters[ComputeRootParameterIndex::FrameData].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    //D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags =
    //	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
    //	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

    D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    auto staticSampler = GetStaticSamplers();

    ZeroDesc(d3dRootSignatureDesc);
    d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
    d3dRootSignatureDesc.pParameters = pd3dRootParameters;
    d3dRootSignatureDesc.NumStaticSamplers = staticSampler.size();
    d3dRootSignatureDesc.pStaticSamplers = staticSampler.data();
    d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

    ID3DBlob* pd3dSignatureBlob = nullptr;
    ID3DBlob* pd3dErrorBlob = nullptr;

    auto result = ::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
    if (FAILED(result)) DebugErrorMessage((const char*)pd3dErrorBlob->GetBufferPointer());
    this->d3dDevice->CreateRootSignature(0, 
        pd3dSignatureBlob->GetBufferPointer(),
        pd3dSignatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&this->postProcessRootSignature));
    TryRelease(pd3dSignatureBlob);
    TryRelease(pd3dErrorBlob);

}

void KG::Renderer::KGDXRenderer::AllocateGBufferHeap()
{
    for (size_t i = 0; i < 5; i++)
    {
        auto index = this->descriptorHeapManager->RequestEmptyIndex();
        DebugNormalMessage(L"디스크립터 힙 G-Buffer용 초기화 : " << index);
    }
}

void KG::Renderer::KGDXRenderer::MoveToNextFrame()
{
    this->swapChainBufferIndex = this->swapChain->GetCurrentBackBufferIndex();
    const UINT64 fenceValue = ++this->fenceValue;
    HRESULT hResult = this->commandQueue->Signal(this->fence, fenceValue);
    if (FAILED(hResult))
    {
        throw hResult;
    }
    if (this->fence->GetCompletedValue() < fenceValue)
    {
        hResult = this->fence->SetEventOnCompletion(fenceValue, this->hFenceEvent);
        ::WaitForSingleObject(this->hFenceEvent, INFINITE);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::KGDXRenderer::GetCurrentRenderTargetHandle() const
{
    D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = this->rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    d3dRtvCPUDescriptorHandle.ptr += (this->swapChainBufferIndex * this->hwFeature.rtvDescriptorSize);
    return d3dRtvCPUDescriptorHandle;
}

ID3D12Resource* KG::Renderer::KGDXRenderer::GetCurrentRenderTarget() const
{
    return this->renderTargetResources[this->swapChainBufferIndex].resource;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 8> GetStaticSamplers()
{
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.

    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
        1, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
        2, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
        3, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
        4, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
        0.0f,                             // mipLODBias
        8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
        5, // shaderRegister
        D3D12_FILTER_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
        0.0f,                              // mipLODBias
        16);                                // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisoCompareClamp(
        6, // shaderRegister
        D3D12_FILTER::D3D12_FILTER_COMPARISON_ANISOTROPIC, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                              // mipLODBias
        16,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);                                // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC linearCompareClamp(
        7, // shaderRegister
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,
        0.0f,
        16,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);                       // maxAnisotropy

    return {
        pointWrap, pointClamp,
        linearWrap, linearClamp,
        anisotropicWrap, anisotropicClamp,
        anisoCompareClamp, linearCompareClamp
    };
}