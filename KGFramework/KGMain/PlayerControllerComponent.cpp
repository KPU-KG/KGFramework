#include "InputManager.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "AnimationComponent.h"
#include "PlayerControllerComponent.h"

void KG::Component::PlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	IComponent::OnCreate(obj);

	auto* spine = this->gameObject->FindChildObject("Spine3"_id);
	spine->GetTransform()->SetScale(0, 0, 0);

	this->characterTransform = this->gameObject->GetComponent<TransformComponent>();
	this->characterAnimation = this->gameObject->GetComponent<AnimationControllerComponent>();

	auto* cameraObject = this->gameObject->FindChildObject("FPCamera"_id);
	this->cameraTransform = cameraObject->GetTransform();
	this->camera = cameraObject->GetComponent<CameraComponent>();

	auto* vectorObject = this->gameObject->FindChildObject("Vector"_id);
	this->vectorAnimation = vectorObject->GetComponent<AnimationControllerComponent>();
	//this->camera = this->gameObject->
}

void KG::Component::PlayerControllerComponent::Update(float elapsedTime)
{
	auto input = KG::Input::InputManager::GetInputManager();
	float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
	speed *= speedValue;
	if ( ImGui::IsAnyItemFocused() )
	{
		return;
	}

	if ( input->IsTouching('1') )
	{
		// -1 : 무한 루프

	}
	if ( input->IsTouching('2') )
	{
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), ANIMSTATE_PLAYING, 0.5f, ANIMLOOP_INF);
	}
	if ( input->IsTouching('3') )
	{
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_right"_id), ANIMSTATE_PLAYING, 0.5f, ANIMLOOP_INF);
	}
	if ( input->IsTouching('4') )
	{
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), ANIMSTATE_PLAYING, 0.5f, ANIMLOOP_INF);
		this->characterAnimation->BlendingAnimation(KG::Utill::HashString("soldier_walk_right"_id), ANIMLOOP_INF, ANIMINDEX_CHANGE);
	}

	if ( input->IsTouching('9') )
	{
		input->SetMouseCapture(false);
	}

	if ( input->IsTouching('0') )
	{
		input->SetMouseCapture(true);
	}



	if ( input->IsTouching('W') )
	{
		this->characterTransform->Translate(this->characterTransform->GetLook() * speed * elapsedTime);
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), ANIMSTATE_PLAYING, 1.0f, ANIMLOOP_INF);
	}
	if ( input->IsTouching('A') )
	{
		this->characterTransform->Translate(this->characterTransform->GetRight() * speed * elapsedTime * -1);
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_left"_id), ANIMSTATE_PLAYING, 1.0f, ANIMLOOP_INF);
	}
	if ( input->IsTouching('S') )
	{
		this->characterTransform->Translate(this->characterTransform->GetLook() * speed * elapsedTime * -1);
		this->characterAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_back"_id), ANIMSTATE_PLAYING, 1.0f, ANIMLOOP_INF);
	}
	if ( input->IsTouching('D') )
	{
		this->characterTransform->Translate(this->characterTransform->GetRight() * speed * elapsedTime);
	}

	if ( input->IsTouching(VK_LBUTTON) && input->GetMouseCapture() )
	{
		this->vectorAnimation->SetAnimation("Vector@Fire.FBX"_id, -1, 1.0f);
		//this->vectorAnimation->
	}

	if ( input->IsTouching(VK_RBUTTON) || input->GetMouseCapture() )
	{
		auto delta = input->GetDeltaMousePosition();
		if ( delta.x )
		{
			this->characterTransform->RotateAxis(Math::up, delta.x * 0.3f);
		}
		if ( delta.y )
		{
			auto euler = this->cameraTransform->GetEulerDegree();
			if ( delta.y < 0 && euler.x > -85.0f || delta.y > 0 && euler.x < 80.0f )
			{
				this->cameraTransform->RotateAxis(Math::right, delta.y * 0.3f);
			}
		}
	}

}

void KG::Component::PlayerControllerComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::PlayerControllerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::PlayerControllerComponent);
}

bool KG::Component::PlayerControllerComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<PlayerControllerComponent>() )
	{
		ImGui::Text("No UI");
	}
	return false;
}


void KG::Component::PlayerControllerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::PlayerControllerComponentSystem::OnPreRender()
{
}
