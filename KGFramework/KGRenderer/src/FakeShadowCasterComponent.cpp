#include "FakeShadowCasterComponent.h"

void KG::Component::FakeShadowCasterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeShadowCasterComponent::OnDestroy()
{
}

void KG::Component::FakeShadowCasterComponent::OnPreRender()
{
}

bool KG::Component::FakeShadowCasterComponent::isPointLightShadow() const
{
    return false;
}

bool KG::Component::FakeShadowCasterComponent::isDirectionalLightShadow() const
{
    return false;
}

bool KG::Component::FakeShadowCasterComponent::isSpotLightShadow() const
{
    return false;
}

KG::Component::LightType KG::Component::FakeShadowCasterComponent::GetTargetLightType() const
{
    return KG::Component::LightType();
}

void KG::Component::FakeShadowCasterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeShadowCasterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}
