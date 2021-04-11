#include "InputManager.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "AnimationComponent.h"
#include "PlayerControllerComponent.h"

static struct SoldierAnimSet
{
	static constexpr auto standing = "Soldier@Standing.fbx"_id;
	static constexpr auto sprint_f = "Soldier@SprintForward.fbx"_id;
	static constexpr auto sprint_fl = "Soldier@SprintForwardLeft.fbx"_id;
	static constexpr auto sprint_fr = "Soldier@Standing.SprintForwardRight"_id;
	static constexpr auto walk_f = "Soldier@WalkForward.fbx"_id;
	static constexpr auto walk_fl = "Soldier@WalkForwardLeft.fbx"_id;
	static constexpr auto walk_fr = "Soldier@WalkForwardRight.fbx"_id;
	static constexpr auto walk_l = "Soldier@WalkLeft.fbx"_id;
	static constexpr auto walk_r = "Soldier@WalkRight.fbx"_id;
	static constexpr auto walk_b = "Soldier@WalkBackward.fbx"_id;
	static constexpr auto walk_bl = "Soldier@WalkBackwardLeft.fbx"_id;
	static constexpr auto walk_br = "Soldier@WalkBackwardRight.fbx"_id;
};

static struct VectorAnimSet
{
	static constexpr auto idle = "Vector@Idle.FBX"_id;
	static constexpr auto reload = "Vector@Reload.FBX"_id;
	static constexpr auto reload_e = "Vector@ReloadEmpty.FBX"_id;
	static constexpr auto fire = "Vector@Fire.FBX"_id;
	static constexpr auto fireAim = "Vector@Fire Aim.FBX"_id;
};



void KG::Component::PlayerControllerComponent::ProcessMove(float elapsedTime)
{
	auto input = KG::Input::InputManager::GetInputManager();
	bool forwardInput = false;
	bool rightInput = false;
	float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
	speed *= speedValue;
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
}

void KG::Component::PlayerControllerComponent::ProcessMoveAnimation(float elapsedTime)
{
	if ( this->forwardValue >= this->inputMinimum )
	{
		//¾Õ
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fr, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fl, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_f, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else if ( this->forwardValue <= -this->inputMinimum )
	{
		//µÚ
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_br, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_bl, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_b, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else
	{
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_r, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_l, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::standing, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
}

void KG::Component::PlayerControllerComponent::ProcessShoot(float elapsedTime)
{
	auto input = KG::Input::InputManager::GetInputManager();
	if ( input->IsTouching(VK_LBUTTON) && input->GetMouseCapture() )
	{
		this->TryShoot(elapsedTime);
	}
}

void KG::Component::PlayerControllerComponent::ProcessMouse(float elapsedTime)
{
	auto input = KG::Input::InputManager::GetInputManager();
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

void KG::Component::PlayerControllerComponent::TryShoot(float elapsedTime)
{
	if ( this->bulletCount <= 0 )
	{
		this->TryReload(elapsedTime);
		return;
	}
	if ( this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::fire )
	{
		DebugNormalMessage("Current Vector Anim Not Fire");
		this->vectorAnimation->SetAnimation(VectorAnimSet::fire, 1, 1.5f);
		this->bulletCount -= 1;
	}
	else if ( this->vectorAnimation->GetCurrentPlayingAnimationTime() > this->bulletRepeatTime )
	{
		DebugNormalMessage("Current Vector Anim is Fire But 0.25Sec later");
		this->vectorAnimation->SetAnimation(VectorAnimSet::fire, 1, 1.5f);
		this->bulletCount -= 1;
	}
}

void KG::Component::PlayerControllerComponent::TryReload(float elapsedTime)
{
	if ( !(this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::fire && this->vectorAnimation->GetCurrentPlayingAnimationTime() <= this->bulletRepeatTime )
		&& this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::reload
		&& this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::reload_e
		)
	{
		if ( this->bulletCount > 0 )
		{
			this->reloadFlag = true;
			this->vectorAnimation->SetAnimation(VectorAnimSet::reload, 1, 1.0f);
		}
		else
		{
			this->reloadFlag = true;
			this->vectorAnimation->SetAnimation(VectorAnimSet::reload_e, 1, 1.0f);
		}
	}
}

bool KG::Component::PlayerControllerComponent::CheckReloading()
{
	if ( this->reloadFlag && (this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::reload || this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::reload_e) )
	{
		if ( this->vectorAnimation->GetCurrentPlayingAnimationTime() >= this->vectorAnimation->GetCurrentPlayingAnimationDuration() * 0.8f )
		{
			this->reloadFlag = false;
			this->bulletCount = 30;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		this->reloadFlag = false;
		return false;
	}
}

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
	auto* input = KG::Input::InputManager::GetInputManager();
	if ( !isActive )
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
	this->ProcessMove(elapsedTime);
	this->ProcessMoveAnimation(elapsedTime);
	this->ProcessMouse(elapsedTime);
	if (! this->CheckReloading() )
	{
		if ( input->GetKeyState('R') == Input::KeyState::Down )
		{
			this->TryReload(elapsedTime);
		}
		else
		{
			this->ProcessShoot(elapsedTime);
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
		ImGui::Checkbox("isActive", &this->isActive);
		ImGui::InputInt("BulletCount", &this->bulletCount);
		ImGui::Checkbox("Reloading", &this->reloadFlag);
	}
	return false;
}


void KG::Component::PlayerControllerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::PlayerControllerComponentSystem::OnPreRender()
{
}
