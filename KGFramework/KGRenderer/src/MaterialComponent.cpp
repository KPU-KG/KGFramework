#include "pch.h"

#include "MaterialComponent.h"
#include "ResourceContainer.h"

#pragma region MaterialComponent

KG::Component::MaterialComponent::MaterialComponent()
{
}

void KG::Component::MaterialComponent::OnCreate(KG::Core::GameObject* obj)
{
	IRenderComponent::OnCreate(obj);
	this->ReloadMaterial();
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

void KG::Component::MaterialComponent::PostMaterial(const KG::Utill::HashString& materialID, UINT slotIndex)
{
	this->materialDescs.descs.emplace_back(false, materialID, slotIndex);
}

void KG::Component::MaterialComponent::PostShader(const KG::Utill::HashString& shaderID, UINT slotIndex)
{
	this->materialDescs.descs.emplace_back(true, shaderID, slotIndex);
}

void KG::Component::MaterialComponent::ReloadMaterial()
{
	for ( auto& i : this->materialDescs.descs )
	{
		if ( i.isRawShader )
		{
			this->InitializeShader(i.materialID, i.slotIndex);
		}
		else
		{
			this->InitializeMaterial(i.materialID, i.slotIndex);
		}
	}
}

void KG::Component::MaterialComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->materialDescs.OnDataLoad(componentElement);
}

void KG::Component::MaterialComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::MaterialComponent);
	this->materialDescs.OnDataSave(componentElement);
}

bool KG::Component::MaterialComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<KG::Component::MaterialComponent>() )
	{
		this->materialDescs.OnDrawGUI();
	}
	return false;
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

#pragma endregion

KG::Component::MateiralDesc::MateiralDesc()
	:materialIDProp("materialID", materialID),
	isRawShaderProp("isRawShader", isRawShader),
	slotIndexProp("slotIndex", slotIndex)
{
}

KG::Component::MateiralDesc::MateiralDesc(bool isRawShader, const KG::Utill::HashString& materialID, UINT slotIndex)
	: MateiralDesc()
{
	this->isRawShader = isRawShader;
	this->materialID = materialID;
	this->slotIndex = slotIndex;
}

KG::Component::MateiralDesc::MateiralDesc(const MateiralDesc& other)
	: MateiralDesc()
{
	this->isRawShader = other.isRawShader;
	this->materialID = other.materialID;
	this->slotIndex = other.slotIndex;
}

KG::Component::MateiralDesc::MateiralDesc(MateiralDesc&& other)
	: MateiralDesc()
{
	this->isRawShader = other.isRawShader;
	this->materialID = other.materialID;
	this->slotIndex = other.slotIndex;
}

KG::Component::MateiralDesc& KG::Component::MateiralDesc::operator=(const MateiralDesc& other)
{
	this->isRawShader = other.isRawShader;
	this->materialID = other.materialID;
	this->slotIndex = other.slotIndex;
	return *this;
}

KG::Component::MateiralDesc& KG::Component::MateiralDesc::operator=(MateiralDesc&& other)
{
	this->isRawShader = other.isRawShader;
	this->materialID = other.materialID;
	this->slotIndex = other.slotIndex;
	return *this;
}

void KG::Component::MateiralDescs::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	auto* obj = objectElement->FirstChildElement("MaterialDesc");
	auto* nextElement = obj->FirstChildElement("Element");
	while ( nextElement != nullptr )
	{
		MateiralDesc desc;
		desc.isRawShaderProp.OnDataLoad(nextElement);
		desc.materialIDProp.OnDataLoad(nextElement);
		desc.slotIndexProp.OnDataLoad(nextElement);
		this->descs.emplace_back(desc);
		nextElement = nextElement->NextSiblingElement("Element");
	}
}

void KG::Component::MateiralDescs::OnDataSave(tinyxml2::XMLElement* objectElement)
{
	auto* materialDescs = objectElement->InsertNewChildElement("MaterialDesc");
	for ( auto& i : this->descs )
	{
		auto* element = materialDescs->InsertNewChildElement("Element");
		i.isRawShaderProp.OnDataSave(element);
		i.materialIDProp.OnDataSave(element);
		i.slotIndexProp.OnDataSave(element);
	}
}

bool KG::Component::MateiralDescs::OnDrawGUI()
{
	if ( ImGui::TreeNodeEx("MateiralDescs", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen) )
	{
		for ( size_t i = 0; i < this->descs.size(); i++ )
		{
			if ( ImGui::TreeNode(std::to_string(i).c_str()) )
			{
				this->descs[i].isRawShaderProp.OnDrawGUI();
				this->descs[i].materialIDProp.OnDrawGUI();
				this->descs[i].slotIndexProp.OnDrawGUI();
				ImGui::TreePop();
			}
		}
		if ( ImGui::SmallButton("Add") )
		{
			this->descs.resize(this->descs.size() + 1);
		}
		ImGui::SameLine();
		if ( ImGui::SmallButton("Delete") )
		{
			this->descs.resize(this->descs.size() - 1 >= 0 ? this->descs.size() - 1 : 0);
		}
		ImGui::TreePop();
	}
	return false;
}
