#include "PlayerControllerComponent.h"

void KG::Component::PlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	IComponent::OnCreate(obj);
}

void KG::Component::PlayerControllerComponent::Update(float elapsedTime)
{
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
