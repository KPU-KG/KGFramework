#include "FakeRender2DComponent.h"

void KG::Component::FakeRender2DComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeRender2DComponent::OnPreRender()
{
}

void KG::Component::FakeRender2DComponent::SetVisible(bool visible)
{
}

bool KG::Component::FakeRender2DComponent::GetVisible() const
{
    return false;
}

void KG::Component::FakeRender2DComponent::ReloadRender()
{
}

void KG::Component::FakeRender2DComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeRender2DComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeRender2DComponent::OnDrawGUI()
{
    return false;
}
