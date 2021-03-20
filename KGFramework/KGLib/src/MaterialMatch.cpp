#include "MaterialMatch.h"
#include "ImguiHelper.h"
void KG::Resource::MaterialMatch::SetDefaultMaterial(const MaterialSet& materials)
{
	this->defaultMaterial = materials;
}

void KG::Resource::MaterialMatch::SetDefaultMaterial(MaterialSet&& materials)
{
	this->defaultMaterial = std::move(materials);
}

void KG::Resource::MaterialMatch::AddMaterial(const KG::Utill::HashString& objectId, const MaterialSet& materials)
{
	this->materialMap.emplace(objectId, materials);
}

void KG::Resource::MaterialMatch::AddMaterial(const KG::Utill::HashString& objectId, MaterialSet&& materials)
{
	this->materialMap.emplace(objectId, std::move(materials));
}

const KG::Resource::MaterialSet& KG::Resource::MaterialMatch::GetMaterial(const KG::Utill::HashString& objectId) const
{
	auto result = this->materialMap.find(objectId);
	return (result == this->materialMap.end()) ? this->defaultMaterial : result->second;
}

void KG::Resource::MaterialMatch::OnDrawGUI()
{
	static KG::Utill::HashString cache;
	if ( ImGui::TreeNodeEx("MaterialMatchSet", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen) )
	{
		ImGui::SameLine();
		if ( ImGui::SmallButton("Clear") )
		{
			this->Clear();
		}
		if ( ImGui::TreeNodeEx("Default Material") )
		{
			for ( size_t i = 0; i < this->defaultMaterial.size(); i++ )
			{
				ImGui::SetNextItemWidth(160);
				ImGui::InputHashString(std::to_string(i).c_str(), &this->defaultMaterial[i]);
			}
			if ( ImGui::SmallButton("Add") )
			{
				this->defaultMaterial.resize(this->defaultMaterial.size() + 1);
			}
			ImGui::SameLine();
			if ( ImGui::SmallButton("Delete") )
			{
				this->defaultMaterial.resize(max(this->defaultMaterial.size() - 1, 0));
			}
			ImGui::TreePop();
		}
		ImGui::SetNextItemWidth(160);
		ImGui::InputHashString("Input", &cache);
		ImGui::SameLine();
		if ( ImGui::SmallButton("Add") )
		{
			this->materialMap.emplace(cache, MaterialSet());
			cache.value = 0;
			cache.srcString = "";
		}
		for ( auto i = this->materialMap.begin(); i != this->materialMap.end(); )
		{
			if ( ImGui::TreeNodeEx(i->first.srcString.c_str()) )
			{
				auto& materialSet = i->second;
				for ( size_t i = 0; i < materialSet.size(); i++ )
				{
					ImGui::SetNextItemWidth(160);
					ImGui::InputHashString(std::to_string(i).c_str(), &materialSet[i]);
				}
				if ( ImGui::SmallButton("Add") )
				{
					materialSet.resize(materialSet.size() + 1);
				}
				ImGui::SameLine();
				if ( ImGui::SmallButton("Delete") )
				{
					materialSet.resize(max(materialSet.size() - 1, 0));
				}
				ImGui::SameLine();
				if ( ImGui::SmallButton("Drop") )
				{
					i = this->materialMap.erase(i);
				}
				else
				{
					++i;
				}
				ImGui::TreePop();
			}
			else
			{
				++i;
			}
		}
		ImGui::TreePop();
	}
}

void KG::Resource::MaterialMatch::Clear()
{
	this->defaultMaterial.clear();
	this->materialMap.clear();
}
