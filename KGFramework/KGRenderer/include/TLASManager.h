#pragma once
#include "D3D12Helper.h"
#include "hash.h"
#include <map>
namespace KG::Renderer
{
    class TLASManager
    {
        ID3D12Resource* tlasResult = nullptr;
        ID3D12Resource* tlasScratch = nullptr;

        ID3D12Resource* instanceData = nullptr;
        D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;

        UINT tlasResultSize = 0;
        UINT tlasScratchSize = 0;
        UINT instanceBufferCount = 0;
    public:
        void Initialize(ID3D12Device* device, UINT reserveSize);
        void AllocateBuffer(ID3D12Device* device, UINT objectSize);
        void UpdateIndex(UINT index, const D3D12_RAYTRACING_INSTANCE_DESC& desc);
        void Build(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList, UINT objectSize);
        D3D12_GPU_VIRTUAL_ADDRESS GetTlasGPUAddress() const;
    };
}