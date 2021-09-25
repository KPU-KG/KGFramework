#pragma once
#include <vector>
#include <map>
#include <string>
#include "D3D12Helper.h"

namespace KG::Renderer
{
    struct InstanceTableElement
    {
        UINT bufferIndex;
    };

    struct InstanceTable
    {
        UINT maxSize = 0;
        ID3D12Resource* table = nullptr;
        UINT* mappedTable = nullptr;

        void ResizeBuffer(ID3D12Device* device, UINT size);

        void UpdateIndex(UINT index, UINT bufferIndex);
        
        void Release();
    };
}