#include "pch.h"
#include "IComponent.h"
#include "GameObject.h"
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
	this->systemInfo.isUsing = true;
}

void KG::Component::IComponent::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
}

void KG::Component::IComponent::OnDataSave(tinyxml2::XMLElement* objectElement)
{
}

void KG::Component::IComponent::OnDrawGUI()
{
}
