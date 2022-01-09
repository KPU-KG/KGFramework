#include "KGRenderQueue.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "DXRRenderScene.h"
#include "RootParameterIndex.h"
#include "RenderTexture.h"

void KG::Renderer::DXRRenderScene::Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSign, DescriptorHeapManager* heap, UINT width, UINT height)
{
    rayGenrationShader.CreateRayGenerationShader(L"Resource/ShaderScript/DXR/DXR_RAY_GEN.hlsl");
    hybridShadowRGShader.CreateRayGenerationShader(L"Resource/ShaderScript/DXR/DXR_RAY_GEN_HYBRID_SHADOW.hlsl");
    hybridAmibentRGShader.CreateRayGenerationShader(L"Resource/ShaderScript/DXR/DXR_RAY_GEN_HYBRID_AMBIENT.hlsl");
    shadowHitMissShader.CreateHitMissShader(L"Resource/ShaderScript/DXR/DXR_SHADOW_HITMISS.hlsl");

    reflectionTraceStateObjects.Initialize(device, globalRootSign);
    pathTraceStateObjects.Initialize(device, globalRootSign);
    shadowTraceStateObjects.Initialize(device, globalRootSign);
    pathTraceStateObjects.AddShader(&rayGenrationShader);
    reflectionTraceStateObjects.AddShader(&hybridShadowRGShader);
    reflectionTraceStateObjects.AddShader(&hybridAmibentRGShader);

    pathTraceStateObjects.AddShader(&shadowHitMissShader);
    shadowHitMissShader.isInitObject = false;
    shadowTraceStateObjects.AddShader(&shadowHitMissShader);
    shadowHitMissShader.isInitObject = false;
    reflectionTraceStateObjects.AddShader(&shadowHitMissShader);

    shaderTables.AddRayGeneration(device);
    shaderTables.AddRayGeneration(device);
    shaderTables.AddRayGeneration(device);

    lights.Initialize(device, 1, heap, width, height);
    tlas.Initialize(device, 100);
}

void KG::Renderer::DXRRenderScene::AddNewRenderJobs(ID3D12Device5* device, KGRenderJob* job)
{
    //Is Not DXR Object
    if (!(job->shader->IsDXRCompatible() && !job->geometry->isFake())) return;
    this->jobs.emplace_back(job);
    auto* dxrShader = job->shader->GetDXRShader();
    if (!dxrShader->isInitObject)
    {
        pathTraceStateObjects.AddShader(dxrShader);
        dxrShader->isInitObject = false;
        reflectionTraceStateObjects.AddShader(dxrShader);
    }
}

void KG::Renderer::DXRRenderScene::UpdateLight(KG::System::UsingComponentIterator<KG::Component::LightComponent> begin, KG::System::UsingComponentIterator<KG::Component::LightComponent> end)
{
    UINT count = 0;
    for (auto i = begin; i != end; ++i, ++count)
    {
        lights.CopyLight(count, i->GetLightType(), i->GetLightData());
    }
}

void KG::Renderer::DXRRenderScene::PrepareRender(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, RenderMode mode)
{
    if (mode == RenderMode::PathTrace)
    {
        pathTraceStateObjects.Rebuild(device);

        shaderTables.PostShadowHit(pathTraceStateObjects.GetHitShaderIndetifier(&shadowHitMissShader));
        shaderTables.PostShadowMiss(pathTraceStateObjects.GetMissShaderIndetifier(&shadowHitMissShader));
        shaderTables.UpdateRay(pathTraceStateObjects.GetRayShaderIndetifier(&this->rayGenrationShader), 0);
    }
    else if (mode == RenderMode::Hybrid)
    {
        //shadowTraceStateObjects.Rebuild(device);
        reflectionTraceStateObjects.Rebuild(device);

        shaderTables.PostShadowHit(reflectionTraceStateObjects.GetHitShaderIndetifier(&shadowHitMissShader));
        shaderTables.PostShadowMiss(reflectionTraceStateObjects.GetMissShaderIndetifier(&shadowHitMissShader));
        shaderTables.UpdateRay(reflectionTraceStateObjects.GetRayShaderIndetifier(&this->hybridShadowRGShader), 1);
        shaderTables.UpdateRay(reflectionTraceStateObjects.GetRayShaderIndetifier(&this->hybridAmibentRGShader), 2);
    }

    auto& currentHitMissStateObject = (mode == RenderMode::PathTrace) ? pathTraceStateObjects : reflectionTraceStateObjects;

    UINT objectSize = 0;
    // Caculate Object Size

    //렌더잡에서 애니메이션 갯수 캐싱
    //캐싱한 갯수 가지고 원본 V, I 가지고 
    //컴퓨트 -> 일단 배칭 개념 안함
    // 컴퓨트 끝나면 blas들 가지고 tlas 구축 및 이거들로 shader table에 추가

    // 컴퓨트하기 -> 애니메이션 인덱스 정하기 -> tlas 구축만 하면 끝
	for (int k = 0; k < this->jobs.size(); k++)
	{
		auto* job = jobs[k];
        if (!job->geometry->IsLoadedDXR())
            job->geometry->LoadToDXR(device, cmdList);
		job->BuildAnimatedBLAS(device, cmdList);
        objectSize += job->notCullIndexStart;
		auto animationSize = job->animationCount;
        shaderTables.UpdateHit(device, currentHitMissStateObject.GetHitShaderIndetifier(job->shader->GetDXRShader()), job, -1);
        shaderTables.UpdateMiss(device, currentHitMissStateObject.GetMissShaderIndetifier(job->shader->GetDXRShader()), job, -1);
		for (size_t i = 0; i < animationSize; i++)
		{
			shaderTables.UpdateHit(device, currentHitMissStateObject.GetHitShaderIndetifier(job->shader->GetDXRShader()), job, i);
			shaderTables.UpdateMiss(device, currentHitMissStateObject.GetMissShaderIndetifier(job->shader->GetDXRShader()), job, i);
		}
    }

    tlas.AllocateBuffer(device, objectSize);
    instanceTable.ResizeBuffer(device, objectSize);
    UINT instanceId = 0;

    // Update ObjectBuffer
	for (int i = 0; i < this->jobs.size(); i++)
	{
		auto* job = jobs[i];
        UINT instanceBufferIndex = 0;

        D3D12_RAYTRACING_INSTANCE_DESC desc;
        KG::Renderer::ZeroDesc(desc);
        desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE;
		
        desc.InstanceMask = 0xFF;

        for (size_t j = 0; j < job->notCullIndexStart; j++)
        {
			auto animationIndex = job->animationCount == 0 ? -1 : j;
			desc.InstanceContributionToHitGroupIndex = shaderTables.GetHitgroupIndex(job, animationIndex) * 2;
            desc.InstanceID = instanceId;
			desc.AccelerationStructure = job->GetBLAS(animationIndex);
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

void KG::Renderer::DXRRenderScene::RenderAllRaytracing(KG::Resource::DXResource& renderTarget, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height)
{
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::AccelerationStructure, this->tlas.GetTlasGPUAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::InstanceIndexData, this->instanceTable.table->GetGPUVirtualAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::LightData, this->lights.GetLightGPUAddress());
    cmdList->SetComputeRootConstantBufferView(DXRRootParameterIndex::Ambient, this->lights.GetAmbientGPUAddress());
    cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::RWTexture, renderTarget.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    this->pathTraceStateObjects.Set(cmdList);

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = shaderTables.GetRayShaderTableGPUAddress();
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.HitGroupTable.StartAddress = shaderTables.GetHitShaderTableGPUAddress();
    rayDesc.HitGroupTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetHitCount();
    rayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.MissShaderTable.StartAddress = shaderTables.GetMissShaderTableGPUAddress();
    rayDesc.MissShaderTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetMissCount();
    rayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.Width = width;
    rayDesc.Height = height;
    rayDesc.Depth = 1;

    renderTarget.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    ApplyBarrierQueue(cmdList);
    cmdList->DispatchRays(&rayDesc);
    renderTarget.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON);
    ApplyBarrierQueue(cmdList);
}

void KG::Renderer::DXRRenderScene::RenderHybridRaytracingShadow(KG::Renderer::RenderTexture& renderTexture, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height)
{
    renderTexture.BarrierTransition(
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    this->reflectionTraceStateObjects.Set(cmdList);
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::AccelerationStructure, this->tlas.GetTlasGPUAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::InstanceIndexData, this->instanceTable.table->GetGPUVirtualAddress());
    cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::LightData, this->lights.GetLightGPUAddress());
    cmdList->SetComputeRootConstantBufferView(DXRRootParameterIndex::Ambient, this->lights.GetAmbientGPUAddress());
    cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::RWTexture, renderTexture.renderTargetResource.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::GBufferHeap, renderTexture.gbufferTextureResources[0].GetDescriptor(DescriptorType::SRV).GetGPUHandle());

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = shaderTables.GetRayShaderTableGPUAddress() + sizeof(ShaderParameter);
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.HitGroupTable.StartAddress = shaderTables.GetHitShaderTableGPUAddress();
    rayDesc.HitGroupTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetHitCount();
    rayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.MissShaderTable.StartAddress = shaderTables.GetMissShaderTableGPUAddress();
    rayDesc.MissShaderTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetMissCount();
    rayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.Width = width;
    rayDesc.Height = height;
    rayDesc.Depth = 1;

    ApplyBarrierQueue(cmdList);
    cmdList->DispatchRays(&rayDesc);
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(renderTexture.renderTargetResource);
    cmdList->ResourceBarrier(1, &barrier);
}

void KG::Renderer::DXRRenderScene::RenderHybridRaytracingAmbient(KG::Renderer::RenderTexture& renderTexture, ID3D12GraphicsCommandList4* cmdList, UINT width, UINT height)
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(renderTexture.renderTargetResource);
    cmdList->ResourceBarrier(1, &barrier);
    this->reflectionTraceStateObjects.Set(cmdList);
    //cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::AccelerationStructure, this->tlas.GetTlasGPUAddress());
    //cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::InstanceIndexData, this->instanceTable.table->GetGPUVirtualAddress());
    //cmdList->SetComputeRootShaderResourceView(DXRRootParameterIndex::LightData, this->lights.GetLightGPUAddress());
    //cmdList->SetComputeRootConstantBufferView(DXRRootParameterIndex::Ambient, this->lights.GetAmbientGPUAddress());
    //cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::RWTexture, renderTexture.renderTargetResource.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    //cmdList->SetComputeRootDescriptorTable(DXRRootParameterIndex::GBufferHeap, renderTexture.gbufferTextureResources[0].GetDescriptor(DescriptorType::SRV).GetGPUHandle());

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = shaderTables.GetRayShaderTableGPUAddress() + sizeof(ShaderParameter) * 2;
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.HitGroupTable.StartAddress = shaderTables.GetHitShaderTableGPUAddress();
    rayDesc.HitGroupTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetHitCount();
    rayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.MissShaderTable.StartAddress = shaderTables.GetMissShaderTableGPUAddress();
    rayDesc.MissShaderTable.SizeInBytes = sizeof(ShaderParameter) * shaderTables.GetMissCount();
    rayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderParameter) / 2;

    rayDesc.Width = width;
    rayDesc.Height = height;
    rayDesc.Depth = 1;

    ApplyBarrierQueue(cmdList);
    cmdList->DispatchRays(&rayDesc);
    renderTexture.BarrierTransition(
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    ApplyBarrierQueue(cmdList);
}
