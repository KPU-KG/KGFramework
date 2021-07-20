#include "KGDX12Resource.h"
#include "KGDXRenderer.h"

KG::Resource::DXResource::DXResource(ID3D12Resource* resource)
{
    this->SetResource(resource);
}

KG::Resource::DXResource::~DXResource()
{
    this->Release();
}

void KG::Resource::DXResource::SetResource(ID3D12Resource* resource)
{
    this->resource = resource;
}

void KG::Resource::DXResource::Release()
{
    //for (auto& i : this->descriptors)
    //{
    //    for (auto& j : i)
    //    {
    //        j.ownerHeap->ReleaseHandleAtIndex(j.HeapIndex);
    //    }
    //}
    //TryRelease(resource);
}

KG::Resource::Descriptor KG::Resource::DXResource::GetDescriptor(DescriptorType type, UINT index) const
{
    return this->descriptors[type][index];
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
    auto& desc = this->AddDescriptor(heap, heap->RequestEmptyIndex(), DescriptorType::SRV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateShaderResourceView(this->resource, &srvDesc, desc.GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc)
{
    auto& desc = this->AddDescriptor(heap, heap->RequestEmptyIndex(), DescriptorType::UAV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateUnorderedAccessView(this->resource, nullptr, &uavDesc, desc.GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc)
{
    auto& desc = this->AddDescriptor(heap, heap->RequestEmptyIndex(), DescriptorType::RTV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateRenderTargetView(this->resource, &rtvDesc, desc.GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc)
{
    auto& desc = this->AddDescriptor(heap, heap->RequestEmptyIndex(), DescriptorType::DSV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateDepthStencilView(this->resource, &dsvDesc, desc.GetCPUHandle());
}

KG::Resource::Descriptor& KG::Resource::DXResource::AddDescriptor(KG::Renderer::DescriptorHeapManager* heap, UINT index, DescriptorType type)
{
    auto& desc = this->descriptors[type].emplace_back();
    desc.ownerHeap = heap;
    desc.HeapIndex = index;
    return desc;
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
    if (!batchedQueue.empty())
    {
        cmdList->ResourceBarrier(batchedQueue.size(), batchedQueue.data());
        batchedQueue.clear();
    }
}

void ApplyBarrierQueue(ID3D12GraphicsCommandList* cmdList)
{
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
}

bool KG::Resource::Descriptor::IsNull() const
{
    return this->ownerHeap == nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE KG::Resource::Descriptor::GetGPUHandle() const
{
    if (ownerHeap)
    {
        return ownerHeap->GetGPUHandle(this->HeapIndex);
    }
    return D3D12_GPU_DESCRIPTOR_HANDLE();
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Resource::Descriptor::GetCPUHandle() const
{
    if (ownerHeap)
    {
        return ownerHeap->GetCPUHandle(this->HeapIndex);
    }
    return D3D12_CPU_DESCRIPTOR_HANDLE();
}
