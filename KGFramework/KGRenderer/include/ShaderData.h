#pragma once
#include <DirectXMath.h>
namespace KG::Renderer
{
	using namespace DirectX;
	struct ObjectData
	{
		XMFLOAT4X4 world;
		unsigned materialIndex;
	};
};