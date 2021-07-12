#include "KGDX12Resource.h"

KG::Resource::DXResource::DXResource(ID3D12Resource* resource)
{
    this->resource = resource;
}

KG::Resource::DXResource::~DXResource()
{
    TryRelease(resource);
}

KG::Resource::Descriptor KG::Resource::DXResource::GetDescriptor(DescriptorType type) const
{
    return this->descriptors[type];
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
    this->SetDescriptorHeap(heap, heap->RequestEmptyIndex(), DescriptorType::SRV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateShaderResourceView(this->resource, &srvDesc, this->GetDescriptor(DescriptorType::SRV).GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc)
{
    this->SetDescriptorHeap(heap, heap->RequestEmptyIndex(), DescriptorType::UAV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateUnorderedAccessView(this->resource, nullptr, &uavDesc, this->GetDescriptor(DescriptorType::UAV).GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc)
{
    this->SetDescriptorHeap(heap, heap->RequestEmptyIndex(), DescriptorType::RTV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateRenderTargetView(this->resource, &rtvDesc, this->GetDescriptor(DescriptorType::RTV).GetCPUHandle());
}

void KG::Resource::DXResource::AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc)
{
    this->SetDescriptorHeap(heap, heap->RequestEmptyIndex(), DescriptorType::DSV);
    auto* device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    device->CreateDepthStencilView(this->resource, &dsvDesc, this->GetDescriptor(DescriptorType::DSV).GetCPUHandle());
}

void KG::Resource::DXResource::SetDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, UINT index, DescriptorType type)
{
    auto& desc = this->descriptors[type];
    desc.ownerHeap = heap;
    desc.HeapIndex = index;
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

