#include "pch.h"
#include "Transform.h"
#include "SceneCameraComponent.h"
#include "InputManager.h"

void KG::Component::SceneCameraComponent::Update(float elapsedTime)
{
	auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
	using namespace KG::Input;
	auto input = InputManager::GetInputManager();
	float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
	speed *= speedValue;
	if ( ImGui::IsAnyItemFocused() )
	{
		return;
	}
	if ( input->IsTouching('W') )
	{
		trans->Translate(trans->GetLook() * speed * elapsedTime);
	}
	if ( input->IsTouching('A') )
	{
		trans->Translate(trans->GetRight() * speed * elapsedTime * -1);
	}
	if ( input->IsTouching('S') )
	{
		trans->Translate(trans->GetLook() * speed * elapsedTime * -1);
	}
	if ( input->IsTouching('D') )
	{
		trans->Translate(trans->GetRight() * speed * elapsedTime);
	}
	if ( input->IsTouching('E') )
	{
		trans->Translate(trans->GetUp() * speed * elapsedTime);
	}
	if ( input->IsTouching('Q') )
	{
		trans->Translate(trans->GetUp() * speed * elapsedTime * -1);
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
			trans->RotateAxis(Math::up, delta.x * 0.3f);
		}
		if ( delta.y )
		{
			trans->RotateAxis(trans->GetRight(), delta.y * 0.3f);
		}
	}
	auto worldPos = trans->GetWorldPosition();
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
