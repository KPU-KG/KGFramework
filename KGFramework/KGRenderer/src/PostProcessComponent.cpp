#include "PostProcessComponent.h"
#include "KGDXRenderer.h"
#include "PostProcess.h"

#include "DescriptorHeapManager.h"

void KG::Component::PostProcessManagerComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::PostProcessManagerComponent::SetActive(int index, bool isActive)
{
    KG::Renderer::KGDXRenderer::GetInstance()->GetPostProcess()->SetActive(index, isActive);
}

void KG::Component::PostProcessManagerComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
    KG::Renderer::KGDXRenderer::GetInstance()->GetPostProcess()->OnDataLoad(componentElement);
}

void KG::Component::PostProcessManagerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::PostProcessManagerComponent);
    KG::Renderer::KGDXRenderer::GetInstance()->GetPostProcess()->OnDataSave(componentElement);
}

bool KG::Component::PostProcessManagerComponent::OnDrawGUI()
{
    if (ImGui::ComponentHeader<PostProcessManagerComponent>())
    {
        KG::Renderer::KGDXRenderer::GetInstance()->GetPostProcess()->OnDrawGUI();
    }
    return false;
}
