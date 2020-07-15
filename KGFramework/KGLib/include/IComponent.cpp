#include "pch.h"
#include "IComponent.h"
using namespace KG::Component;

IComponent::IComponent()
{
	OnCreate();
	this->SetActive(true);
}

void IComponent::SetActive(bool isActive)
{
	if (this->isActive != isActive)
		return;

	this->isActive = isActive;

	if (this->isActive)
	{
		this->OnStart();
	}
	else
	{
		this->OnEnd();
	}
}

void KG::Component::IComponent::Destroy()
{
	this->isDestroy = true;
	this->OnDestroy();
}

bool KG::Component::IComponent::IsDestroy()
{
	return this->isDestroy;
}
