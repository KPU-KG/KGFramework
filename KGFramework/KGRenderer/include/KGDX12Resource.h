#pragma once
#include <vector>
#include <array>
#include "D3D12Helper.h"
#include "DescriptorHeapManager.h"
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
        
        bool IsNull() const;

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
    };

    struct DXResource
    {
        DXResource() = default;
        DXResource(ID3D12Resource* resource);

        ~DXResource();

        ID3D12Resource* resource = nullptr;
        void SetResource(ID3D12Resource* resource);

        std::array<std::vector<KG::Resource::Descriptor>, KG::Resource::DescriptorType::DescriptorTypeCount> descriptors;

        void Release();
        
        Descriptor GetDescriptor(DescriptorType type, UINT index = 0) const;

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_RENDER_TARGET_VIEW_DESC rtvDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc);

        Descriptor& AddDescriptor(KG::Renderer::DescriptorHeapManager* heap, UINT index, DescriptorType type);

        // Barrier
        D3D12_RESOURCE_STATES currentState;
        void AddTransitionQueue(D3D12_RESOURCE_STATES next);
        
        inline static std::vector<D3D12_RESOURCE_BARRIER> batchedQueue;
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
using KG::Resource::DescriptorType;
void ApplyBarrierQueue(ID3D12GraphicsCommandList* cmdList);