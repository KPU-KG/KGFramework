#include "TLASManager.h"

void KG::Renderer::TLASManager::Initialize(ID3D12Device* device, UINT reserveSize)
{
    this->AllocateBuffer(device, reserveSize);
}

void KG::Renderer::TLASManager::AllocateBuffer(ID3D12Device* device, UINT objectSize)
{
    if (objectSize <= this->instanceBufferCount) return;
    if (this->instanceData)
    {
        this->instanceData->Unmap(0, nullptr);
        this->instanceData->Release();
        this->instanceData = nullptr;
        this->instances = nullptr;
    }
    instanceBufferCount = objectSize;
    this->instanceData = CreateUploadHeapBuffer(device, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * this->instanceBufferCount);
    this->instanceData->Map(0, nullptr, (void**)&this->instances);
}

void KG::Renderer::TLASManager::UpdateIndex(UINT index, const D3D12_RAYTRACING_INSTANCE_DESC& desc)
{
    memcpy(this->instances + index, &desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
}

void KG::Renderer::TLASManager::Build(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, UINT objectSize)
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
    ZeroDesc(inputs);
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    inputs.NumDescs = objectSize;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.InstanceDescs = this->instanceData->GetGPUVirtualAddress();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild;
    ZeroDesc(prebuild);

    //대충 업데이트할거면 기존 -> 새 버퍼 로 이중버퍼 쓰는 느낌
    device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);
    if (this->tlasScratchSize < prebuild.ScratchDataSizeInBytes)
    {
        TryRelease(this->tlasScratch);
        this->tlasScratchSize = prebuild.ScratchDataSizeInBytes;
        this->tlasScratch = CreateASBufferResource(device, cmdList, this->tlasScratchSize, D3D12_RESOURCE_STATE_COMMON);
    }

    if (this->tlasResultSize < prebuild.ResultDataMaxSizeInBytes)
    {
        TryRelease(this->tlasResult);
        this->tlasResultSize = prebuild.ResultDataMaxSizeInBytes;
        this->tlasResult = CreateASBufferResource(device, cmdList, this->tlasResultSize, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
    ZeroDesc(buildDesc);
    buildDesc.DestAccelerationStructureData = this->tlasResult->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = this->tlasScratch->GetGPUVirtualAddress();
    buildDesc.Inputs = inputs;

    cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::TLASManager::GetTlasGPUAddress() const
{
    return this->tlasResult->GetGPUVirtualAddress();
}