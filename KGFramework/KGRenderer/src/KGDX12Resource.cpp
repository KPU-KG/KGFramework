#include "KGDX12Resource.h"

KG::Resource::DXResource::DXResource(ID3D12Resource* resource)
{
    this->resource = resource;
}

KG::Resource::DXResource::~DXResource()
{
    TryRelease(resource);
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
    this->ownerDescHeap = heap;
    descHeapIndex = this->ownerDescHeap->RequestEmptyIndex();
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateShaderResourceView(this->resource, &srvDesc, this->GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc)
{
    this->ownerDescHeap = heap;
    descHeapIndex = this->ownerDescHeap->RequestEmptyIndex();
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateUnorderedAccessView(this->resource, nullptr, &uavDesc, this->GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, UINT index)
{
    this->ownerDescHeap = heap;
    descHeapIndex = index;
}

bool KG::Resource::DXResource::IsInDescHeap() const
{
    return this->ownerDescHeap != nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE KG::Resource::DXResource::GetGPUHandle() const
{
    if (ownerDescHeap)
    {
        return ownerDescHeap->GetGPUHandle(this->descHeapIndex);
    }
    return D3D12_GPU_DESCRIPTOR_HANDLE();
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Resource::DXResource::GetCPUHandle() const
{
    if (ownerDescHeap)
    {
        return ownerDescHeap->GetCPUHandle(this->descHeapIndex);
    }
    return D3D12_CPU_DESCRIPTOR_HANDLE();
}

void KG::Resource::DXResource::AddTransitionQueue(D3D12_RESOURCE_STATES next)
{
    if (next != currentState)
    {
        batchedQueue.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource, currentState, next));
        currentState = next;
    }
}

void KG::Resource::DXResource::ApplyBarrierQueue(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->ResourceBarrier(batchedQueue.size(), batchedQueue.data());
    batchedQueue.clear();
}

