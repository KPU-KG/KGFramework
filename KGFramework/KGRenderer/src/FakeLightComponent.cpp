#include "FakeLightComponent.h"

void KG::Component::FakeLightComponent::SetLightPower(float lightPower)
{
}

void KG::Component::FakeLightComponent::SetDirectionalLight(const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction)
{
}

void KG::Component::FakeLightComponent::SetPointLight(const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd)
{
}

void KG::Component::FakeLightComponent::SetSpotLight(const DirectX::XMFLOAT3& strength, float depth, float Phi, float Theta, float fallOff)
{
}

static KG::Component::LightData nullData;

KG::Component::DirectionalLightRef KG::Component::FakeLightComponent::GetDirectionalLightRef()
{
    return DirectionalLightRef(nullData);
}

KG::Component::PointLightRef KG::Component::FakeLightComponent::GetPointLightRef()
{
    return PointLightRef(nullData);
}

KG::Component::SpotLightRef KG::Component::FakeLightComponent::GetSpotLightRef()
{
    return SpotLightRef(nullData);
}

void KG::Component::FakeLightComponent::OnPreRender()
{
}

KG::Component::LightType KG::Component::FakeLightComponent::GetLightType() const
{
    return LightType();
}

void KG::Component::FakeLightComponent::SetVisible(bool visible)
{
}

void KG::Component::FakeLightComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeLightComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeLightComponent::OnDrawGUI()
{
    return false;
}
