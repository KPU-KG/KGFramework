#pragma once
#include <d3d12.h>
#include "IComponent.h"

namespace KG::Component
{
    class IDXRenderComponent
    {
    public:
        virtual void OnRender(ID3D12GraphicsCommandList* commandList) {};
    };
};
