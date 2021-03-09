#include "pch.h"
#include "GeometryComponent.h"
#include "ResourceContainer.h"

KG::Component::GeometryComponent::GeometryComponent()
	:
	geometryIDProp("GeometryID", geometryID, true),
	subMeshIndexProp("SubMeshIndex", subMeshIndex, true),
	slotIndexProp("SlotIndex", slotIndex, true)
{
}

void KG::Component::GeometryComponent::OnCreate(KG::Core::GameObject* obj)
{
	IRenderComponent::OnCreate(obj);
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->geometrys.size() < slotIndex + 1 )
	{
		this->geometrys.resize(slotIndex + 1);
	}
	this->geometrys[slotIndex] = inst->LoadGeometry(geometryID, subMeshIndex);
}

bool KG::Component::GeometryComponent::HasBone() const
{
	bool result = false;
	for ( auto* i : geometrys )
	{
		if ( i->HasBone() )
			return true;
	}
	return false;
}

void KG::Component::GeometryComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->subMeshIndexProp.OnDataLoad(componentElement);
	this->slotIndexProp.OnDataLoad(componentElement);
	this->subMeshIndexProp.OnDataLoad(componentElement);
}

void KG::Component::GeometryComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::GeometryComponent);
	this->subMeshIndexProp.OnDataSave(componentElement);
	this->slotIndexProp.OnDataSave(componentElement);
	this->subMeshIndexProp.OnDataSave(componentElement);
}
