#pragma once
#include <DirectXMath.h>
#include <array>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Component
{
	struct LightData
	{
		DirectX::XMFLOAT3 Strength;
		float lightPower = 1.0f;
		DirectX::XMFLOAT3 Direction;
		float pad0;
		DirectX::XMFLOAT3 Position;
		float pad1;
		float FalloffStart;
		float FalloffEnd;
		float Phi;
		float Theta;
		DirectX::XMFLOAT3 Up;
		float pad2;
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

	struct SpotLightRef
	{
		DirectX::XMFLOAT3& Strength;
		float& Theta;
		float& Phi;
		float& depth;
		float& falloff;
		SpotLightRef( LightData& light )
			: Strength( light.Strength ), falloff( light.FalloffEnd ), Phi( light.Phi ), Theta( light.Theta ), depth( light.FalloffStart )
		{
		};
	};

	enum class LightType
	{
		DirectionalLight,
		PointLight,
		SpotLight
	};

	class DLL ILightComponent : public IRenderComponent
	{
	public:
		virtual void SetLightPower( float lightPower ) = 0;
		virtual void SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction ) = 0;
		virtual void SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd ) = 0;
		virtual void SetSpotLight( const DirectX::XMFLOAT3& strength, float depth, float Phi, float Theta, float fallOff ) = 0;
		virtual DirectionalLightRef GetDirectionalLightRef() = 0;
		virtual PointLightRef GetPointLightRef() = 0;
		virtual SpotLightRef GetSpotLightRef() = 0;
		virtual void OnPreRender() override = 0;
        virtual LightType GetLightType() const = 0;
		virtual void SetVisible( bool visible ) = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};

	REGISTER_COMPONENT_ID( ILightComponent );
};
