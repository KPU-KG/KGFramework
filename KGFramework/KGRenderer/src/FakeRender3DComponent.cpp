#include "FakeRender3DComponent.h"

void KG::Component::FakeRender3DComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeRender3DComponent::OnPreRender()
{
}

void KG::Component::FakeRender3DComponent::SetVisible(bool visible)
{
}

bool KG::Component::FakeRender3DComponent::GetVisible() const
{
    return false;
}

void KG::Component::FakeRender3DComponent::SetReflectionProbe(ICubeCameraComponent* probe)
{
}

void KG::Component::FakeRender3DComponent::ReloadRender()
{
}

void KG::Component::FakeRender3DComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeRender3DComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeRender3DComponent::OnDrawGUI()
{
    return false;
}
