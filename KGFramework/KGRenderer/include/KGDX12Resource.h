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
        DSV
    };
    struct Descriptor
    {

    };
    struct DXResource
    {
        DXResource(ID3D12Resource* resource);

        ~DXResource();

        ID3D12Resource* resource = nullptr;

        //Desc Heap
        KG::Renderer::DescriptorHeapManager* ownerDescHeap = nullptr;
        UINT descHeapIndex;
        
        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc);

        void AddOnDescriptorHeap(KG::Renderer::DescriptorHeapManager* heap, UINT index);

        bool IsInDescHeap() const;

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;

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