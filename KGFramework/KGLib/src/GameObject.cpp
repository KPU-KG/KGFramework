#include <algorithm>
#include <string>
#include <cstring>
#include "GameObject.h"
#include "Transform.h"
#include "Scene.h"

KG::Core::GameObject* KG::Core::GameObject::InternalFindChildObject(const KG::Utill::HashString& tag) const
{
	if ( this->tag == tag )
	{
		return const_cast<KG::Core::GameObject*>(this);
	}
	else
	{
		auto* ch = this->GetChild();
		auto* sib = this->GetSibling();

		if ( ch != nullptr )
		{
			auto* res = ch->InternalFindChildObject(tag);
			if ( res != nullptr ) return res;
		}

		if ( sib != nullptr )
		{
			auto* res = sib->InternalFindChildObject(tag);
			if ( res != nullptr ) return res;
		}
	}
	return nullptr;
}

void KG::Core::GameObject::InternalMatchBoneToObject(const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones) const
{
	for ( size_t i = 0; i < tags.size(); i++ )
	{
		if ( tags[i] == this->tag )
		{
			bones[i] = const_cast<KG::Core::GameObject*>(this);
		}
	}

	auto* ch = this->GetChild();
	auto* sib = this->GetSibling();

	if ( ch != nullptr ) ch->InternalMatchBoneToObject(tags, bones);

	if ( sib != nullptr ) sib->InternalMatchBoneToObject(tags, bones);
}

void KG::Core::GameObject::InternalDestroy()
{
	this->isDestroy = true;
	auto* child = this->GetChild();
	auto* sib = this->GetSibling();
	if ( child )
		child->InternalDestroy();
	if ( sib )
		sib->InternalDestroy();
}

KG::Core::GameObject::GameObject()
	: tagProp("Tag", this->tag)
{
}

bool KG::Core::GameObject::IsDestroy() const
{
	return this->isDestroy;
}

void KG::Core::GameObject::Destroy()
{
	this->isDestroy = true;
	auto* child = this->GetChild();
	if ( child )
		child->InternalDestroy();
}

void KG::Core::GameObject::SetOwnerScene(KG::Core::Scene* ownerScene)
{
	this->ownerScene = ownerScene;
}

void KG::Core::GameObject::SetInstanceID(UINT32 instanceID)
{
	this->instanceID = instanceID;
}

KG::Component::TransformComponent* KG::Core::GameObject::GetTransform() const
{
	return this->GetComponent<KG::Component::TransformComponent>();
}

inline KG::Core::GameObject* KG::Core::GameObject::GetChild() const
{
	return this->GetTransform() && this->GetTransform()->hasChild() ? this->GetTransform()->GetChild()->GetGameObject() : nullptr;
}

inline KG::Core::GameObject* KG::Core::GameObject::GetSibling() const
{
	return this->GetTransform() && this->GetTransform()->hasNextSibiling() ? this->GetTransform()->GetNextsibiling()->GetGameObject() : nullptr;
}

KG::Core::GameObject* KG::Core::GameObject::FindChildObject(const KG::Utill::HashString& tag) const
{
	if ( this->tag == tag )
	{
		return const_cast<KG::Core::GameObject*>(this);
	}
	else
	{
		auto* ch = this->GetChild();

		if ( ch != nullptr )
		{
			auto* res = ch->InternalFindChildObject(tag);
			if ( res != nullptr ) return res;
		}
	}
	return nullptr;
}

KG::Core::Scene* KG::Core::GameObject::GetScene() const
{
	return this->ownerScene;
}

UINT32 KG::Core::GameObject::GetInstanceID() const
{
	return this->instanceID;
}

void KG::Core::GameObject::MatchBoneToObject(const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones) const
{
	for ( size_t i = 0; i < tags.size(); i++ )
	{
		if ( tags[i] == this->tag )
		{
			bones[i] = const_cast<KG::Core::GameObject*>(this);
		}
	}

	auto* ch = this->GetChild();
	if ( ch != nullptr ) ch->InternalMatchBoneToObject(tags, bones);
}

void KG::Core::GameObject::SaveToPrefab(const std::string& name)
{
}

void KG::Core::GameObject::SaveToFile(const std::string& filePath)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLDeclaration* dec1 = doc.NewDeclaration();
	tinyxml2::XMLElement* objectElement = doc.NewElement("SavedGameObject");
	this->OnPrefabSave(objectElement);
	doc.LinkEndChild(dec1);
	doc.LinkEndChild(objectElement);
	auto e = doc.SaveFile(filePath.c_str());
}

void KG::Core::GameObject::LoadToFile(const std::string& filePath)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filePath.c_str());
	auto* objectElement = doc.FirstChildElement("SavedGameObject")->FirstChildElement("GameObject");
	this->OnDataLoad(objectElement);
	auto e = doc.SaveFile(filePath.c_str());
}

void KG::Core::GameObject::OnPrefabLoad(tinyxml2::XMLElement* objectElement)
{
	this->tagProp.OnDataLoad(objectElement);

	auto* childObjectElement = objectElement->FirstChildElement("GameObject");
	if ( childObjectElement )
	{
		auto* childObject = this->GetScene()->CreateNewTransformObject();
		auto* childTrans = childObject->GetTransform();
		childTrans->SetParent(this->GetTransform());
		this->GetTransform()->SetChild(childTrans);
		childObject->OnDataLoad(childObjectElement);
	}

	auto* siblingObjectElement = objectElement->NextSiblingElement("GameObject");
	if ( siblingObjectElement )
	{
		auto* siblingObject = this->GetScene()->CreateNewTransformObject();
		auto siblingTrans = siblingObject->GetTransform();
		siblingTrans->SetParent(this->GetTransform()->GetParent());
		this->GetTransform()->SetNextSibiling(siblingTrans);
		siblingObject->OnDataLoad(siblingObjectElement);
	}

	auto* componentElement = objectElement->FirstChildElement("Component");
	while ( componentElement != nullptr )
	{
		KG::Utill::HashString componentId = componentElement->UnsignedAttribute("hash_id");
		auto* component = this->ownerScene->GetComponentProvider()->GetComponent(componentId);
		component->OnDataLoad(componentElement);
		this->AddComponentWithID(componentId, component);
		componentElement = componentElement->NextSiblingElement("Component");
	}
}

void KG::Core::GameObject::OnPrefabSave(tinyxml2::XMLElement* parentElement)
{
	auto* objectElement = parentElement->InsertNewChildElement("GameObject");
	objectElement->SetAttribute("instanceId", this->instanceID);
	this->tagProp.OnDataSave(objectElement);
	for ( auto& i : this->components.container )
	{
		i.second->OnDataSave(objectElement);
	}

	if ( this->GetTransform() )
	{
		if ( this->GetChild() )
		{
			this->GetChild()->OnDataSave(objectElement);
		}
	}
}

void KG::Core::GameObject::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	this->tagProp.OnDataLoad(objectElement);
	auto* childObjectElement = objectElement->FirstChildElement("GameObject");
	if ( childObjectElement )
	{
		auto instId = childObjectElement->UnsignedAttribute("instanceId");
		auto* childObject =  this->GetScene()->CreateNewTransformObject(instId);
		auto* childTrans = childObject->GetTransform();
		childTrans->SetParent(this->GetTransform());
		this->GetTransform()->SetChild(childTrans);
		childObject->OnDataLoad(childObjectElement);
	}

	auto* siblingObjectElement = objectElement->NextSiblingElement("GameObject");
	if ( siblingObjectElement )
	{
		auto instId = siblingObjectElement->UnsignedAttribute("instanceId");
		auto* siblingObject = this->GetScene()->CreateNewTransformObject(instId);
		auto siblingTrans = siblingObject->GetTransform();
		siblingTrans->SetParent(this->GetTransform()->GetParent());
		this->GetTransform()->SetNextSibiling(siblingTrans);
		siblingObject->OnDataLoad(siblingObjectElement);
	}

	auto* componentElement = objectElement->FirstChildElement("Component");
	while ( componentElement != nullptr )
	{
		KG::Utill::HashString componentId = componentElement->UnsignedAttribute("hash_id");
		if ( componentId.value == KG::Component::ComponentID<KG::Component::TransformComponent>::id() )
		{
			this->GetTransform()->OnDataLoad(componentElement);
		}
		else
		{
			auto* component = this->ownerScene->GetComponentProvider()->GetComponent(componentId);
			component->OnDataLoad(componentElement);
			this->AddComponentWithID(componentId, component);
		}
		componentElement = componentElement->NextSiblingElement("Component");
	}

}

void KG::Core::GameObject::LoadHierarchy(tinyxml2::XMLElement* parentElement, bool prefabMode)
{
}

void KG::Core::GameObject::LoadComponent(tinyxml2::XMLElement* parentElement)
{
}


void KG::Core::GameObject::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* objectElement = parentElement->InsertNewChildElement("GameObject");
	objectElement->SetAttribute("instanceId", this->instanceID);
	this->tagProp.OnDataSave(objectElement);
	for ( auto& i : this->components.container )
	{
		i.second->OnDataSave(objectElement);
	}

	if ( this->GetTransform() )
	{
		if ( this->GetChild() )
		{
			this->GetChild()->OnDataSave(objectElement);
		}
		if ( this->GetSibling() )
		{
			this->GetSibling()->OnDataSave(parentElement);
		}
	}
}


bool KG::Core::GameObject::OnDrawGUI()
{
	static char searchBuffer[256] = {};
	if ( ImGui::CollapsingHeader("Object Info", ImGuiTreeNodeFlags_DefaultOpen) )
	{
		ImGui::InputHashString("Tag", &this->tag);
		if ( ImGui::Button("Add New Component") )
		{
			ImGui::OpenPopup("new_comp");
		}
		if ( ImGui::BeginPopup("new_comp") )
		{
			ImGui::Text("List");
			if ( !ImGui::IsAnyItemActive() )
				ImGui::SetKeyboardFocusHere();
			ImGui::InputText("Search", searchBuffer, 256);
			ImGui::Separator();
			auto* prov = this->GetScene()->GetComponentProvider();
			for ( auto& i : prov->getterFunctions )
			{
				if ( strlen(searchBuffer) == 0 || (strstr(i.first.srcString.c_str(), searchBuffer) != nullptr) )
				{
					if ( ImGui::Selectable(i.first.srcString.c_str()) )
					{
						searchBuffer[0] = '\0';
						KG::Component::IComponent* obj = i.second();
						obj->Reserve(this);
						this->temporalComponents.emplace_back(std::make_pair(i.first, obj));
					}
				}
			}
			ImGui::EndPopup();
		}
	}
	if ( ImGui::CollapsingHeader("Component List", ImGuiTreeNodeFlags_DefaultOpen) )
	{
		ImGui::BulletText("Temporal Components : %d", this->temporalComponents.size());
		ImGui::SameLine();
		if ( ImGui::SmallButton("Add All") )
		{
			for ( auto i = this->temporalComponents.begin(); i != this->temporalComponents.end(); ++i )
			{
				this->AddComponentWithID(i->first, i->second);
			}
			temporalComponents.clear();
		}
		ImGui::Indent();
		int id = 0;
		for ( auto i = this->temporalComponents.begin(); i != this->temporalComponents.end(); )
		{
			bool skip = false;
			ImGui::PushID(id++);
			i->second->DrawGUI(this->currentGUIContext);
			{
				if ( ImGui::SmallButton("Add") )
				{
					this->AddComponentWithID(i->first, i->second);
					i = this->temporalComponents.erase(i);
					skip = true;
				}
				ImGui::SameLine();
				if ( ImGui::SmallButton("Delete") )
				{
					i->second->UnReserve();
					i = this->temporalComponents.erase(i);
					skip = true;
				}
			}
			ImGui::PopID();
			ImGui::Separator();
			if ( !skip )
			{
				++i;
			}
		}
		ImGui::Unindent();

		ImGui::BulletText("Using Components : %d", this->components.container.size());
		ImGui::Indent();
		for ( auto& i : this->components.container )
		{
			ImGui::PushID(id++);
			i.second->DrawGUI(this->currentGUIContext);
			{
				ImGui::SmallButton("Delete");
			}
			ImGui::PopID();
			ImGui::Separator();
		}
		ImGui::Unindent();
	}
	return false;
}
