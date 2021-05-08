#pragma once
#include <DirectXMath.h>
#include <array>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "ILightComponent.h"
#include "IDXRenderComponent.h"

namespace KG::Renderer
{
	class Shader;
	class Geometry;
	struct KGRenderJob;
};

namespace KG::Component
{
	class TransformComponent;

	struct ShadowData
	{
		UINT shadowMapIndex[4];
		DirectX::XMFLOAT4X4 shadowMatrix[4];
	};

	class LightComponent : public ILightComponent, IDXRenderComponent
	{
		KG::Renderer::KGRenderJob* renderJob = nullptr;
		TransformComponent* transform = nullptr;
		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		bool isDirty = true;
		LightType lightType = LightType::DirectionalLight;
		LightData light = {};
		ShadowData shadow;
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
		void InitializeDirectionalLight();
		void InitializePointLight();
		void InitializeSpotLight();
	public:
		LightComponent();
		virtual void SetLightPower( float lightPower ) override;
		virtual void SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction ) override;
		virtual void SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd ) override;
		virtual void SetSpotLight( const DirectX::XMFLOAT3& strength, float depth, float Phi, float Theta, float fallOff ) override;
		virtual DirectionalLightRef GetDirectionalLightRef() override;
		virtual PointLightRef GetPointLightRef() override;
		virtual SpotLightRef GetSpotLightRef() override;

		void UpdateChanged();

		void SetShadowCasterTextureIndex( UINT index );
		void SetShadowMatrix( const DirectX::XMFLOAT4X4 matrix );
		void SetShadowCascadeMatrix( const std::array<DirectX::XMFLOAT4X4, 4>& cascades );

		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
        virtual LightType GetLightType() const override { return this->lightType; }
		virtual void SetVisible( bool visible ) override;
	private:
		KG::Core::SerializableEnumProperty<KG::Component::LightType> lightTypeProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> strengthProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> directionProp;
		KG::Core::SerializableProperty<float> fallOffStartProp;
		KG::Core::SerializableProperty<float> fallOffEndProp;
		KG::Core::SerializableProperty<float> depthProp;
		KG::Core::SerializableProperty<float> phiProp;
		KG::Core::SerializableProperty<float> thetaProp;
		KG::Core::SerializableProperty<float> fallOffProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;
	};

    REGISTER_COMPONENT_ID_REPLACE( LightComponent, ILightComponent);
};
