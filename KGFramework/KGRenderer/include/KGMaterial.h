#pragma once
#include <d3d12.h>
#include <map>
#include <memory>
#include "hash.h"
#include "ResourceMetadata.h"
#include "KGShader.h"

namespace KG::Renderer
{
    class MaterialCore
    {
        bool isSurfaceShader = false;
        KG::Utill::HashString id;
        Shader* normalShader;
        //¥Î√Ê DXRShader* dxrShader
    };

    class MaterialInstance
    {
        MaterialCore* core;
        KG::Utill::HashString id;
        UINT materialIndex = -1;
        UINT hitShaderIndex = -1;
    };
}