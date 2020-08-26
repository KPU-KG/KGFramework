#pragma once
#include <DirectXMath.h>
namespace KG::Renderer
{
	using namespace DirectX;
	struct ObjectData
	{
		union
		{
			struct object
			{
				XMFLOAT4X4 world;
				unsigned materialIndex;
			} object;
			struct light
			{
				XMFLOAT3 Strength;
				float FalloffStart;
				XMFLOAT3 Direction;
				float FalloffEnd;
				XMFLOAT3 Position;
				float SpotPower;
			} light;
		};
	};
};