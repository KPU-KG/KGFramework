#pragma once
#include <d3d12.h>
#include "d3dx12.h"
namespace KG::Renderer
{
	class RenderTexture
	{
		ID3D12Resource* renderTarget;
		D3D12_RESOURCE_STATES defaultState;
	};
}