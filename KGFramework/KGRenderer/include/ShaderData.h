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
				UINT materialIndex;
				UINT environmentMapIndex;
				UINT pad1;
				UINT pad2;
				XMFLOAT4X3 pad3;
			} object;
			struct light
			{
				XMFLOAT3 Strength;
				float FalloffStart;
				XMFLOAT3 Direction;
				float FalloffEnd;
				XMFLOAT3 Position;
				float SpotPower;
				UINT shadowMapIndex;
				XMFLOAT3 pad1;
				XMFLOAT4X4 shadowMatrix;
			} light;
		};
	};
};