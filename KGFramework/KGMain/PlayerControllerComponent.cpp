#include "InputManager.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "AnimationComponent.h"
#include "PlayerControllerComponent.h"
#include "PhysicsComponent.h"

void KG::Component::PlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	IComponent::OnCreate(obj);

	auto* spine = this->gameObject->FindChildObject("Spine3"_id);
	spine->GetTransform()->SetScale(0, 0, 0);

	this->thisTransform = this->gameObject->GetComponent<TransformComponent>();
	this->thisAnimation = this->gameObject->GetComponent<AnimationControllerComponent>();

	auto* cameraObject = this->gameObject->FindChildObject("FPCamera"_id);
	this->cameraTransform = cameraObject->GetTransform();
	this->camera = cameraObject->GetComponent<CameraComponent>();
	this->cameraAnimation = cameraObject->GetComponent<AnimationControllerComponent>();
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
		this->thisAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_left"_id), 0.5f, -1);
	}
	if ( input->IsTouching('2') )
	{
		this->thisAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), 0.5f, -1);
	}
	if ( input->IsTouching('3') )
	{
		this->thisAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_right"_id), 0.5f, -1);
	}
	if ( input->IsTouching('4') )
	{
		this->thisAnimation->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), 0.5f, -1);
		this->thisAnimation->BlendingAnimation(KG::Utill::HashString("soldier_walk_right"_id), -1, -1);
		this->thisAnimation->BlendingAnimation(KG::Utill::HashString("soldier_walk_right"_id), -1, 0);
	}

	auto dynamicRigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();

	if ( input->IsTouching('W') )
	{
		if (dynamicRigid) {
			auto dir = this->thisTransform->GetWorldLook();
			dir.y = 0;
			dynamicRigid->Move(dir, speed * 3 * elapsedTime);
		}
		else
			this->thisTransform->Translate(this->thisTransform->GetLook() * speed * elapsedTime);
	}
	if ( input->IsTouching('A') )
	{
		if (dynamicRigid) {
			auto dir = this->thisTransform->GetWorldRight();
			dir.y = 0;
			dynamicRigid->Move(dir * -1, speed * 3 * elapsedTime);
		}
		else
			this->thisTransform->Translate(this->thisTransform->GetRight() * speed * elapsedTime * -1);
	}
	if ( input->IsTouching('S') )
	{
		if (dynamicRigid) {
			auto dir = this->thisTransform->GetWorldLook();
			dir.y = 0;
			dynamicRigid->Move(dir * -1, speed * 3 * elapsedTime);
		}
		else
			this->thisTransform->Translate(this->thisTransform->GetLook() * speed * elapsedTime * -1);
	}
	if (input->IsTouching('D'))
	{
		if (dynamicRigid) {
			auto dir = this->thisTransform->GetWorldRight();
			dir.y = 0;
			dynamicRigid->Move(dir, speed * 3 * elapsedTime);
		}
		else
			this->thisTransform->Translate(this->thisTransform->GetRight() * speed * elapsedTime);
	}

	if ( input->IsTouching(VK_RBUTTON) )
	{
		auto delta = input->GetDeltaMousePosition();
		if ( delta.x )
		{
			this->thisTransform->RotateAxis(Math::up, delta.x * 0.3f);
		}
		if ( delta.y )
		{
			this->cameraTransform->RotateAxis(Math::right, delta.y * 0.3f);
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
