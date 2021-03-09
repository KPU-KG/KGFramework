#include "pch.h"

#include "MaterialComponent.h"
#include "ResourceContainer.h"

#pragma region MaterialComponent

KG::Component::MaterialComponent::MaterialComponent()
	:
	isRawShaderProp("isRawShader", this->isRawShader, true),
	materialIDProp("MaterialID", this->materialID, true),
	slotIndexProp("SlotIndex", this->slotIndex, true)
{
}

void KG::Component::MaterialComponent::OnCreate(KG::Core::GameObject* obj)
{
	IRenderComponent::OnCreate(obj);
	if ( this->isRawShader )
	{
		this->InitializeShader(this->materialID, this->slotIndex);
	}
	else
	{
		this->InitializeMaterial(this->materialID, this->slotIndex);
	}
}

void KG::Component::MaterialComponent::InitializeMaterial(const KG::Utill::HashString& materialID, UINT slotIndex)
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	auto [index, shaderId] = inst->LoadMaterial(materialID);
	if ( this->materialIndexs.size() < slotIndex + 1 )
	{
		this->materialIndexs.resize(slotIndex + 1);
	}
	this->materialIndexs[slotIndex] = index;
	this->InitializeShader(shaderId, slotIndex);
}

void KG::Component::MaterialComponent::InitializeShader(const KG::Utill::HashString& shaderID, UINT slotIndex)
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->shaders.size() < slotIndex + 1 )
	{
		this->shaders.resize(slotIndex + 1);
	}
	this->shaders[slotIndex] = inst->LoadShader(shaderID);
}

unsigned KG::Component::MaterialComponent::GetMaterialIndex(UINT slotIndex) const
{
	return this->materialIndexs.size() != 0 ? this->materialIndexs[slotIndex] : 0;
}

void KG::Component::MaterialComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->isRawShaderProp.OnDataLoad(componentElement);
	this->materialIDProp.OnDataLoad(componentElement);
	this->slotIndexProp.OnDataLoad(componentElement);
}

void KG::Component::MaterialComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::MaterialComponent);
	this->isRawShaderProp.OnDataSave(componentElement);
	this->materialIDProp.OnDataSave(componentElement);
	this->slotIndexProp.OnDataSave(componentElement);
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

#pragma endregion