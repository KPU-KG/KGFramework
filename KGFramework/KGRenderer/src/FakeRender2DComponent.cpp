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

void KG::Component::FakeRenderSpriteComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeRenderSpriteComponent::OnPreRender()
{
}

void KG::Component::FakeRenderSpriteComponent::SetVisible(bool visible)
{
}

bool KG::Component::FakeRenderSpriteComponent::GetVisible() const
{
    return false;
}

void KG::Component::FakeRenderSpriteComponent::ReloadRender()
{
}

void KG::Component::FakeRenderSpriteComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeRenderSpriteComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeRenderSpriteComponent::OnDrawGUI()
{
    return false;
}
