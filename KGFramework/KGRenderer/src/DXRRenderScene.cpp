#include "KGRenderQueue.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "DXRRenderScene.h"
#include "RootParameterIndex.h"

void KG::Renderer::DXRRenderScene::Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSign)
{
    rayGenrationShader.CreateRayGenerationShader(L"Resource/ShaderScript/DXR/DXR_RAY_GEN.hlsl");
    stateObjects.Initialize(device, globalRootSign);
    stateObjects.AddShader(&rayGenrationShader);
    shaderTables.AddRayGeneration(device);
    lights.Initialize(device, 10);
    tlas.Initialize(device, 100);
}

void KG::Renderer::DXRRenderScene::AddNewRenderJobs(ID3D12Device5* device, KGRenderJob* job)
{
    //Is Not DXR Object
    if (!(job->shader->IsDXRCompatible() && !job->geometry->isFake())) return;
    this->jobs.emplace_back(job);
    auto* dxrShader = job->shader->GetDXRShader();
    if (!dxrShader->isInitObject) stateObjects.AddShader(dxrShader);
    shaderTables.AddHit(device, job);
    shaderTables.AddMiss(device, job);
}

void KG::Renderer::DXRRenderScene::UpdateLight(KG::System::UsingComponentIterator<KG::Component::LightComponent> begin, KG::System::UsingComponentIterator<KG::Component::LightComponent> end)
{
    UINT count = 0;
    for (auto i = begin; i != end; ++i, ++count)
    {
        lights.CopyLight(count, i->GetLightType(), i->GetLightData());
    }
}

void KG::Renderer::DXRRenderScene::PrepareRender(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{
    stateObjects.Rebuild(device);
    shaderTables.UpdateRay(stateObjects.GetRayShaderIndetifier(&this->rayGenrationShader));

    UINT objectSize = 0;
    // Caculate Object Size
    for (auto* job : jobs)
    {
        if (!job->geometry->IsLoadedDXR())
            job->geometry->LoadToDXR(device, cmdList);
        objectSize += job->notCullIndexStart;
        shaderTables.UpdateHit(stateObjects.GetHitShaderIndetifier(job->shader->GetDXRShader()), job);
        shaderTables.UpdateMiss(stateObjects.GetMissShaderIndetifier(job->shader->GetDXRShader()), job);
    }

    tlas.AllocateBuffer(device, objectSize);
    instanceTable.ResizeBuffer(device, objectSize);
    UINT instanceId = 0;

    // Update ObjectBuffer
    for (auto* job : jobs)
    {
        UINT instanceBufferIndex = 0;

        D3D12_RAYTRACING_INSTANCE_DESC desc;
        KG::Renderer::ZeroDesc(desc);
        desc.InstanceContributionToHitGroupIndex = shaderTables.GetHitgroupIndex(job);;
        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        desc.InstanceMask = 0xFF;
        desc.AccelerationStructure = job->geometry->GetBLAS();

        for (size_t i = 0; i < job->notCullIndexStart; i++)
        {
            desc.InstanceID = instanceId;
            *(XMFLOAT3X4*)desc.Transform = job->matrixs[instanceBufferIndex];
            tlas.UpdateIndex(instanceId, desc);
            instanceTable.UpdateIndex(instanceId, instanceBufferIndex);

            instanceId++;
            instanceBufferIndex++;
        }
    }
    tlas.Build(device, cmdList, objectSize);
    lights.UpdateAmbient();
}

void KG::Renderer::DXRRenderScene::Render(KG::Resource::DXResource& renderTarget, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height)
{
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::AccelerationStructure, this->tlas.GetTlasGPUAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::InstanceIndexData, this->instanceTable.table->GetGPUVirtualAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::LightData, this->lights.GetLightGPUAddress());
    cmdList->SetComputeRootConstantBufferView(DXRRootParameterIndex::Ambient, this->lights.GetAmbientGPUAddress());
    cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::RWTexture, renderTarget.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    this->stateObjects.Set(cmdList);

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = shaderTables.GetRayShaderTableGPUAddress();
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderParameter);

    rayDesc.HitGroupTable.StartAddress = shaderTables.GetHitShaderTableGPUAddress();
    rayDesc.HitGroupTable.SizeInBytes = sizeof(ShaderParameter);
    rayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderParameter);

    rayDesc.MissShaderTable.StartAddress = shaderTables.GetMissShaderTableGPUAddress();
    rayDesc.MissShaderTable.SizeInBytes = sizeof(ShaderParameter);
    rayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderParameter);

    rayDesc.Width = width;
    rayDesc.Height = height;
    rayDesc.Depth = 1;

    //renderTarget.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    //ApplyBarrierQueue(cmdList);
    cmdList->DispatchRays(&rayDesc);

}
