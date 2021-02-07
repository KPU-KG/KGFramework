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
				UINT pad0;
				UINT pad1;
			} object;
			struct light
			{
				XMFLOAT3 Strength;
				float FalloffStart;
				XMFLOAT3 Direction;
				float FalloffEnd;
				XMFLOAT3 Position;
				float SpotPower;
				XMFLOAT4 pad0;
				XMFLOAT4 pad1;
			} light;
		};
	};

	struct ShadowLightData
	{
		union
		{
			//struct ForwardLight
			//{

			//};
			struct DeferredShadow
			{
				UINT shadowMapIndex[4];
				DirectX::XMFLOAT4X4 shadowMatrix[4];
			} shadow;
		};
	};
};