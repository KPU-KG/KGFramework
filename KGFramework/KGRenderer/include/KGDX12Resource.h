#pragma once
#include "D3D12Helper.h"
#include "DescriptorHeapManager.h"
#include "KGDXRenderer.h"
#include "d3dx12.h"   
namespace KG::Resource
{
    enum DescriptorType
    {
        UAV,
        SRV,
        RTV,
        DSV,
        DescriptorTypeCount
    };

    struct Descriptor
    {
        UINT HeapIndex = -1;
        KG::Renderer::DescriptorHeapManager* ownerHeap = nullptr;
        
        bool IsNull() const
        {
            return this->ownerHeap == nullptr;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const
        {
            if (ownerHeap)
            {
                return ownerHeap->GetGPUHandle(this->HeapIndex);
            }
            return D3D12_GPU_DESCRIPTOR_HANDLE();
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const
        {
            if (ownerHeap)
            {
                return ownerHeap->GetCPUHandle(this->HeapIndex);
            }
            return D3D12_CPU_DESCRIPTOR_HANDLE();
        }
    };

    struct DXResource
    {
        DXResource(ID3D12Resource* resource);

        ~DXResource();

        ID3D12Resource* resource = nullptr;

        std::array<Descriptor, DescriptorType::DescriptorTypeCount> descriptors;
        
        Descriptor GetDescriptor(DescriptorType type) const;

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc);

        void SetDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, UINT index, DescriptorType type);

        // Barrier
        D3D12_RESOURCE_STATES currentState;
        void AddTransitionQueue(D3D12_RESOURCE_STATES next);
        
        static std::vector<D3D12_RESOURCE_BARRIER> batchedQueue;
        static void ApplyBarrierQueue(ID3D12GraphicsCommandList* cmdList);

        operator ID3D12Resource* ()
        {
            return resource;
        }
        ID3D12Resource* operator->()
        {
            return resource;
        }
    };
}