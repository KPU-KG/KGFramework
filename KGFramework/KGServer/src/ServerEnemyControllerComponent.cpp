#include "ServerEnemyControllerComponent.h"
#include "Transform.h"
#include "imgui/imgui.h"

KG::Component::SEnemyControllerComponent::SEnemyControllerComponent()
	:
	speedProp("Speed", this->speed),
	rangeProp("Range", this->range)
{
}


void KG::Component::SEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	this->center = this->gameObject->GetTransform()->GetWorldPosition();
}

void KG::Component::SEnemyControllerComponent::Update(float elapsedTime)
{
	
}

bool KG::Component::SEnemyControllerComponent::OnDrawGUI()
{
	// if (ImGui::ComponentHeader<KG::Component::SEnemyControllerComponent>()) {
	// 	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
	// 	if (this->isUsing()) {
	// 		ImGui::TextDisabled("Center (%f, %f, %f)", center.x, center.y, center.z);
	// 	}
	// 	this->speedProp.OnDrawGUI();
	// 	this->rangeProp.OnDrawGUI();
	// }
	return false;
}

void KG::Component::SEnemyControllerComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	// this->rangeProp.OnDataLoad(componentElement);
	// this->speedProp.OnDataLoad(componentElement);
}

void KG::Component::SEnemyControllerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	// auto* componentElement = parentElement->InsertNewChildElement("Component");
	// ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::SEnemyControllerComponent);
	// this->rangeProp.OnDataSave(componentElement);
	// this->speedProp.OnDataSave(componentElement);
}

bool KG::Component::SEnemyControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

void KG::Component::SEnemyControllerComponentSystem::OnPostUpdate(float elapsedTime)
{

}

void KG::Component::SEnemyControllerComponentSystem::OnPreRender()
{

}