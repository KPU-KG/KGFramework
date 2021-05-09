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
				DirectX::XMFLOAT3 Strength;
				float lightPower;
				DirectX::XMFLOAT3 Direction;
				float pad0;
				DirectX::XMFLOAT3 Position;
				float pad1;
				float FalloffStart;
				float FalloffEnd;
				float Phi;
				float Theta;
				XMFLOAT3 Up;
				float pad2;
			} light;
			struct particle
			{
				DirectX::XMFLOAT3 position;
				float rotation;
				DirectX::XMFLOAT3 speed;
				float rotationSpeed;
				DirectX::XMFLOAT2 size;
				UINT materialIndex;
				float pad1;
				DirectX::XMFLOAT4 color;
				double startTime;
				float lifeTime;
				float pad0;
			}particle;
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