#include "PostProcessComponent.h"
#include "KGDXRenderer.h"
#include "PostProcess.h"

#include "DescriptorHeapManager.h"

void KG::Component::PostProcessManagerComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::PostProcessManagerComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::PostProcessManagerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::PostProcessManagerComponent::OnDrawGUI()
{
    KG::Renderer::KGDXRenderer::GetInstance()->GetPostProcess()->OnDrawGUI();
    return false;
}
