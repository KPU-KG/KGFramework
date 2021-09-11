#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGDX12Resource.h"
#include "KGDXRenderer.h"
namespace KG::Renderer::RTX
{
    struct ShaderTableElement
    {
        UINT paramSize;
        void* data;
    };


    class ShaderTable
    {
        UINT shaderParamMaxSize = sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        ID3D12Resource* resource = nullptr;
    };
}