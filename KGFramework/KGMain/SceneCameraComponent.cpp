#include "pch.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "SceneCameraComponent.h"
#include "InputManager.h"

using namespace KG::Math::Literal;

void KG::Component::SceneCameraComponent::OnCreate(KG::Core::GameObject* obj)
{
    IComponent::OnCreate(obj);
    this->transform = gameObject->GetComponent<KG::Component::TransformComponent>();
    this->camera = gameObject->GetComponent<KG::Component::CameraComponent>();
}

void KG::Component::SceneCameraComponent::Update(float elapsedTime)
{
    using namespace KG::Input;
    auto input = InputManager::GetInputManager();
    float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
    speed *= speedValue;
    if ( ImGui::IsAnyItemFocused() || !this->camera->isMainCamera )
    {
        return;
    }
    if ( input->IsTouching('W') )
    {
        this->transform->Translate(this->transform->GetLook() * speed * elapsedTime);
    }
    if ( input->IsTouching('A') )
    {
        this->transform->Translate(this->transform->GetRight() * speed * elapsedTime * -1);
    }
    if ( input->IsTouching('S') )
    {
        this->transform->Translate(this->transform->GetLook() * speed * elapsedTime * -1);
    }
    if ( input->IsTouching('D') )
    {
        this->transform->Translate(this->transform->GetRight() * speed * elapsedTime);
    }
    if ( input->IsTouching('E') )
    {
        this->transform->Translate(this->transform->GetUp() * speed * elapsedTime);
    }
    if ( input->IsTouching('Q') )
    {
        this->transform->Translate(this->transform->GetUp() * speed * elapsedTime * -1);
    }

    if ( input->IsTouching('O') )
    {
        this->speedValue -= 0.5f;
    }
    if ( input->IsTouching('P') )
    {
        this->speedValue += 0.5f;
    }



    if ( input->IsTouching(VK_RBUTTON) )
    {
        auto delta = input->GetDeltaMousePosition();
        if ( delta.x )
        {
            this->transform->RotateAxis(Math::up, delta.x * 0.3f);
        }
        if ( delta.y )
        {
            this->transform->RotateAxis(this->transform->GetRight(), delta.y * 0.3f);
        }
    }
    auto worldPos = this->transform->GetWorldPosition();
}

void KG::Component::SceneCameraComponent::PostUpdateFunction(const UpdateFunctor& functor)
{
}

void KG::Component::SceneCameraComponent::OnDestroy()
{
}

void KG::Component::SceneCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::SceneCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
    auto* componentElement = parentElement->InsertNewChildElement("Component");
    ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::SceneCameraComponent);
}

bool KG::Component::SceneCameraComponent::OnDrawGUI()
{
    if ( ImGui::ComponentHeader<SceneCameraComponent>() )
    {
        ImGui::Text("No UI");
    }
    return false;
}
