#pragma once
#include "D3D12Helper.h"
#include <d3d12.h>
#include <array>
#include <DirectXMath.h>

namespace KG::Resource
{
	using namespace DirectX;
	constexpr UINT MAX_COUNT_BONE = 64;
	struct BoneData
	{
		std::array<XMFLOAT4X4, MAX_COUNT_BONE> offsetMatrixs;
	};
	struct AnimationData
	{
		std::array<XMFLOAT4X4, MAX_COUNT_BONE> currentTransforms;
	};
}