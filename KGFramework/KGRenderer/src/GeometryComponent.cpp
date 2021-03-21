#include "pch.h"
#include "GeometryComponent.h"
#include "ResourceContainer.h"

KG::Component::GeometryComponent::GeometryComponent()
{
}

void KG::Component::GeometryComponent::OnCreate(KG::Core::GameObject* obj)
{
	IRenderComponent::OnCreate(obj);
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	for ( auto& i : this->geometryDescs.materialDescs )
	{
		this->InitializeGeometry(i.geometryID, i.subMeshIndex, i.slotIndex);
	}
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

void KG::Component::GeometryComponent::InitializeGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->geometrys.size() < slotIndex + 1 )
	{
		this->geometrys.resize(slotIndex + 1);
	}
	this->geometrys[slotIndex] = inst->LoadGeometry(geometryID, subMeshIndex);
}

void KG::Component::GeometryComponent::AddGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
{
	this->geometryDescs.materialDescs.emplace_back(
		geometryID, subMeshIndex, slotIndex
	);
}

void KG::Component::GeometryComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->geometryDescs.OnDataLoad(componentElement);
}

void KG::Component::GeometryComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::GeometryComponent);
	this->geometryDescs.OnDataSave(componentElement);
}

bool KG::Component::GeometryComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<KG::Component::GeometryComponent>() )
	{
		this->geometryDescs.OnDrawGUI();
	}
	return false;
}

KG::Component::GeometryDesc::GeometryDesc()
	:geometryIDProp("geometryID", geometryID),
	subMeshIndexProp("subMeshIndex", subMeshIndex),
	slotIndexProp("slotIndex", slotIndex)
{
}

KG::Component::GeometryDesc::GeometryDesc(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
	:GeometryDesc()
{
	this->geometryID = geometryID;
	this->subMeshIndex = subMeshIndex;
	this->slotIndex = slotIndex;
}

KG::Component::GeometryDesc::GeometryDesc(const GeometryDesc& other )
	:GeometryDesc()
{
	this->geometryID = other.geometryID;
	this->subMeshIndex = other.subMeshIndex;
	this->slotIndex = other.slotIndex;
}

KG::Component::GeometryDesc::GeometryDesc(GeometryDesc&& other)
	:GeometryDesc()
{
	this->geometryID = other.geometryID;
	this->subMeshIndex = other.subMeshIndex;
	this->slotIndex = other.slotIndex;
}

KG::Component::GeometryDesc& KG::Component::GeometryDesc::operator=(const GeometryDesc& other)
{
	this->geometryID = other.geometryID;
	this->subMeshIndex = other.subMeshIndex;
	this->slotIndex = other.slotIndex;
	return *this;
}

KG::Component::GeometryDesc& KG::Component::GeometryDesc::operator=(GeometryDesc&& other)
{
	this->geometryID = other.geometryID;
	this->subMeshIndex = other.subMeshIndex;
	this->slotIndex = other.slotIndex;
	return *this;
}

void KG::Component::GeometryDescs::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	auto* obj = objectElement->FirstChildElement("GeometryDescs");
	auto* nextElement = obj->FirstChildElement("Element");
	while ( nextElement != nullptr )
	{
		GeometryDesc desc;
		desc.geometryIDProp.OnDataLoad(nextElement);
		desc.slotIndexProp.OnDataLoad(nextElement);
		desc.subMeshIndexProp.OnDataLoad(nextElement);
		this->materialDescs.emplace_back(desc);
		nextElement = nextElement->NextSiblingElement("Element");
	}
}

void KG::Component::GeometryDescs::OnDataSave(tinyxml2::XMLElement* objectElement)
{
	auto* materialDescs = objectElement->InsertNewChildElement("GeometryDescs");
	for ( auto& i : this->materialDescs )
	{
		auto* element = materialDescs->InsertNewChildElement("Element");
		i.geometryIDProp.OnDataSave(element);
		i.slotIndexProp.OnDataSave(element);
		i.subMeshIndexProp.OnDataSave(element);
	}
}

bool KG::Component::GeometryDescs::OnDrawGUI()
{
	if ( ImGui::TreeNodeEx("GeometryDescs", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen) )
	{
		for ( size_t i = 0; i < this->materialDescs.size(); i++ )
		{
			if ( ImGui::TreeNode(std::to_string(i).c_str()) )
			{
				this->materialDescs[i].geometryIDProp.OnDrawGUI();
				this->materialDescs[i].subMeshIndexProp.OnDrawGUI();
				this->materialDescs[i].slotIndexProp.OnDrawGUI();
				ImGui::TreePop();
			}
		}
		if ( ImGui::SmallButton("Add") )
		{
			this->materialDescs.resize(this->materialDescs.size() + 1);
		}
		ImGui::SameLine();
		if ( ImGui::SmallButton("Delete") )
		{
			this->materialDescs.resize(this->materialDescs.size() - 1 >= 0 ? this->materialDescs.size() - 1 : 0);
		}
		ImGui::TreePop();
	}
	return false;
}
