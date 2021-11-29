#pragma once
#include "D3D12Helper.h"
#include "ShaderTableManager.h"
#include "StateObjectManager.h"
#include "TLASManager.h"
#include "InstanceTable.h"
#include "KGDX12Resource.h"
#include "LightTable.h"
#include "KGShader.h"
namespace KG::Renderer
{
    struct KGRenderJob;
    struct RenderTexture;


    enum class RenderMode
    {
        PathTrace,
        Hybrid,
        None = PathTrace
    };
    class DXRRenderScene
    {
        StateObjectManager pathTraceStateObjects;
        StateObjectManager shadowTraceStateObjects;
        StateObjectManager reflectionTraceStateObjects;
        ShaderTableManager shaderTables;
        InstanceTable instanceTable;
        TLASManager tlas;
        LightTable lights;
        std::vector<KGRenderJob*> jobs;
        DXRShader rayGenrationShader;
        DXRShader hybridShadowRGShader;
        DXRShader hybridAmibentRGShader;

        DXRShader shadowHitMissShader;
    public:
        void Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSign, DescriptorHeapManager* heap, UINT width, UINT height);
        void AddNewRenderJobs(ID3D12Device5* device, KGRenderJob* job);
        void UpdateLight(KG::System::UsingComponentIterator<KG::Component::LightComponent> begin, KG::System::UsingComponentIterator<KG::Component::LightComponent> end);
        void PrepareRender(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, RenderMode mode = RenderMode::None);
        void RenderAllRaytracing(KG::Resource::DXResource& renderTarget, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height);
        void RenderHybridRaytracingShadow(KG::Renderer::RenderTexture& rt, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height);
        void RenderHybridRaytracingAmbient(KG::Renderer::RenderTexture& rt, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height);

    };
}