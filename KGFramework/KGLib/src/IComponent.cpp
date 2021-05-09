#include "pch.h"
#include "IComponent.h"
#include "GameObject.h"
#include "imguiHelper.h"
using namespace KG::Component;

IComponent::IComponent()
{
}

void IComponent::SetActive(bool isActive)
{
	if (this->isActive != isActive)
		return;

	this->isActive = isActive;

	if (this->isActive)
	{
		this->OnActive();
	}
	else
	{
		this->OnDisactive();
	}
}

void KG::Component::IComponent::OnDestroy() 
{
	//this->systemInfo.isUsing = true;???
}

void KG::Component::IComponent::Create(KG::Core::GameObject* gameObject)
{
	this->gameObject = gameObject;
	PostUse();
	this->OnCreate(gameObject);
}

void KG::Component::IComponent::Reserve(KG::Core::GameObject* gameObject)
{
	this->gameObject = gameObject;
	PostReserve();
}

void KG::Component::IComponent::Update(float timeElapsed)
{
}

void KG::Component::IComponent::OnDebugUpdate(float timeElasped)
{
}

void KG::Component::IComponent::Destroy()
{
	this->gameObject->DeleteComponent(this);
}

void KG::Component::IComponent::InternalDestroy()
{
	this->systemInfo.isUsing = false;
	this->UnReserve();
	this->OnDestroy();
}

void KG::Component::IComponent::UnReserve()
{
	this->systemInfo.isReserved = false;
}

void KG::Component::IComponent::PostUse()
{
	this->systemInfo.isUsing = true;
	this->PostReserve();
}

void KG::Component::IComponent::PostReserve()
{
	this->systemInfo.isReserved = true;
}

bool KG::Component::IComponent::isReserved() const
{
	return this->systemInfo.isUsing || this->systemInfo.isReserved;
}

bool KG::Component::IComponent::isUsing() const
{
	return this->systemInfo.isUsing == true;
}

void KG::Component::IComponent::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
}

void KG::Component::IComponent::OnDataSave(tinyxml2::XMLElement* objectElement)
{
}

bool KG::Component::IComponent::OnDrawGUI()
{
	if ( ImGui::CollapsingHeader("Not Implement UI Component") )
	{
		ImGui::Text("this component has not UI");
		ImGui::Text("please override component OnDrawGui method");
	}
	return false;
}
