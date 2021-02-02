#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	class Shader;
	class Geometry;
	struct KGRenderJob;
};

namespace KG::Component
{
	class TransformComponent;
	struct LightData
	{
		DirectX::XMFLOAT3 Strength;
		float FalloffStart;
		DirectX::XMFLOAT3 Direction;
		float FalloffEnd;
		DirectX::XMFLOAT3 Position;
		float SpotPower;
		UINT shadowMapIndex;
		DirectX::XMFLOAT3 pad1;
		DirectX::XMFLOAT4X4 shadowMatrix;
	};

	struct PointLightRef
	{
		DirectX::XMFLOAT3& Strength;
		float& FalloffStart;
		float& FalloffEnd;
		PointLightRef( LightData& light )
			: Strength( light.Strength ), FalloffStart( light.FalloffStart ), FalloffEnd( light.FalloffEnd )
		{
		};
	};

	struct DirectionalLightRef
	{
		DirectX::XMFLOAT3& Strength;
		DirectX::XMFLOAT3& Direction;
		DirectionalLightRef( LightData& light )
			: Strength( light.Strength ), Direction( light.Direction )
		{
		};
	};

	enum class LightType
	{
		DirectionalLight,
		PointLight,
		SpotLight
	};

	class DLL LightComponent : public IRenderComponent
	{
		KG::Renderer::KGRenderJob* renderJob = nullptr;
		TransformComponent* transform = nullptr;
		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		bool isDirty = true;
		LightType lightType = LightType::DirectionalLight;
		LightData light;
		KG::Renderer::Shader* currentShader = nullptr;
		KG::Renderer::Geometry* currentGeometry = nullptr;
		void RegisterTransform( TransformComponent* transform );

		inline static KG::Renderer::Shader* directionalLightShader = nullptr;
		inline static KG::Renderer::Shader* spotLightShader = nullptr;
		inline static KG::Renderer::Shader* pointLightShader = nullptr;

		inline static KG::Renderer::Geometry* directionalLightGeometry = nullptr;
		inline static KG::Renderer::Geometry* spotLightGeometry = nullptr;
		inline static KG::Renderer::Geometry* pointLightGeometry = nullptr;

		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
	public:
		void SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction );
		void SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd );
		DirectionalLightRef GetDirectionalLightRef();
		PointLightRef GetPointLightRef();

		void UpdateChanged();

		void SetShadowCasterTextureIndex( UINT index );
		void SetShadowMatrix( const DirectX::XMFLOAT4X4 matrix );

		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
		auto GetLightType() const { return this->lightType; }
		void SetVisible( bool visible );
	};

	REGISTER_COMPONENT_ID( LightComponent );
};
