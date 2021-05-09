#pragma once
#include <DirectXMath.h>
#include <array>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "ILightComponent.h"

namespace KG::Component
{
	class FakeLightComponent : public ILightComponent
	{
    public:
        // ILightComponent을(를) 통해 상속됨
        virtual void SetLightPower(float lightPower) override;
        virtual void SetDirectionalLight(const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction) override;
        virtual void SetPointLight(const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd) override;
        virtual void SetSpotLight(const DirectX::XMFLOAT3& strength, float depth, float Phi, float Theta, float fallOff) override;
        virtual DirectionalLightRef GetDirectionalLightRef() override;
        virtual PointLightRef GetPointLightRef() override;
        virtual SpotLightRef GetSpotLightRef() override;
        virtual void OnPreRender() override;
        virtual LightType GetLightType() const override;
        virtual void SetVisible(bool visible) override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };

    REGISTER_COMPONENT_ID_REPLACE( FakeLightComponent, ILightComponent);
};
