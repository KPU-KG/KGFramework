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
	if ( ImGui::IsAnyItemFocused() || !this->camera->isMainCamera )
	{
		return;
	}


	if ( input->IsTouching('9') )
	{
		input->SetMouseCapture(false);
	}

	if ( input->IsTouching('0') )
	{
		input->SetMouseCapture(true);
	}


	bool forwardInput = false;
	bool rightInput = false;

	if ( input->IsTouching('W') )
	{
		this->forwardValue += inputRatio * +1 * elapsedTime;
		forwardInput = true;
	}
	if ( input->IsTouching('S') )
	{
		this->forwardValue += inputRatio * -1 * elapsedTime;
		forwardInput = true;
	}
	if ( input->IsTouching('D') )
	{
		this->rightValue += inputRatio * +1 * elapsedTime;
		rightInput = true;
	}
	if ( input->IsTouching('A') )
	{
		this->rightValue += inputRatio * -1 * elapsedTime;
		rightInput = true;
	}

	if ( !forwardInput )
	{
		if ( abs(forwardValue) > this->inputMinimum )
		{
			forwardValue += inputRetRatio * ((forwardValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			forwardValue = 0.0f;
		}
	}

	if ( !rightInput )
	{
		if ( abs(rightValue) > this->inputMinimum )
		{
			rightValue += inputRetRatio * ((rightValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			rightValue = 0.0f;
		}
	}
	forwardValue = KG::Math::Clamp(forwardValue, -1.0f, 1.0f);
	rightValue = KG::Math::Clamp(rightValue, -1.0f, 1.0f);


	if ( abs(this->forwardValue) >= this->inputMinimum )
	{
		this->characterTransform->Translate(this->characterTransform->GetLook() * speed * elapsedTime * this->forwardValue);
	}
	if ( abs(this->rightValue) >= this->inputMinimum )
	{
		this->characterTransform->Translate(this->characterTransform->GetRight() * speed * elapsedTime * this->rightValue);
	}


	if ( this->forwardValue >= this->inputMinimum )
	{
		//¾Õ
		this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_f, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_fr, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_fl, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else if ( this->forwardValue <= -this->inputMinimum )
	{
		//µÚ
		this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_b, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_br, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_bl, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else
	{
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_r, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_walk_l, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ChangeAnimation(this->anim_soldier_standing, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}

	if ( input->IsTouching(VK_LBUTTON) && input->GetMouseCapture() )
	{
		this->vectorAnimation->SetAnimation("Vector@Fire.FBX"_id, 1, 1.5f);
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
