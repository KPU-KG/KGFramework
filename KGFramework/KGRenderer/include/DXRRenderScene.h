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
    class DXRRenderScene
    {
        StateObjectManager stateObjects;
        ShaderTableManager shaderTables;
        InstanceTable instanceTable;
        TLASManager tlas;
        LightTable lights;
        std::vector<KGRenderJob*> jobs;
        DXRShader rayGenrationShader;
    public:
        void Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSign);
        void AddNewRenderJobs(ID3D12Device5* device, KGRenderJob* job);
        void UpdateLight(KG::System::UsingComponentIterator<KG::Component::LightComponent> begin, KG::System::UsingComponentIterator<KG::Component::LightComponent> end);
        void PrepareRender(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
        void Render(KG::Resource::DXResource& renderTarget, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height);
    };
}