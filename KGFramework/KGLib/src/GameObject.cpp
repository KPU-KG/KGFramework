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

bool KG::Core::GameObject::IsDestroy() const
{
	return this->isDestroy;
}

void KG::Core::GameObject::Destroy()
{
	this->isDestroy = true;
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
	return this->GetTransform()->hasChild() ? this->GetTransform()->GetChild()->GetGameObject() : nullptr;
}

inline KG::Core::GameObject* KG::Core::GameObject::GetSibling() const
{
	return this->GetTransform()->hasSibiling() ? this->GetTransform()->GetNextsibiling()->GetGameObject() : nullptr;
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

void KG::Core::GameObject::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	auto* comp = objectElement->FirstChildElement();
	while ( comp )
	{
		KG::Utill::HashString componentName = comp->UnsignedAttribute("hash_id");
		this->ownerScene->GetComponentProvider()->AddComponentToObject(componentName, this)->OnDataLoad(comp);
		comp = objectElement->NextSiblingElement();
	}
}

void KG::Core::GameObject::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* objectElement = parentElement->InsertNewChildElement("GameObject");
	for ( auto& i : this->components.container )
	{
		auto* compElement = objectElement->InsertNewChildElement("Component");
		compElement->SetAttribute("hash_id", i.first.value);
		i.second->OnDataSave(compElement);
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
						obj->PostReserve();
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
			for ( auto i = this->temporalComponents.begin(); i != this->temporalComponents.end(); ++i)
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
