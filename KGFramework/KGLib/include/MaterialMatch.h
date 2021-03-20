#pragma once
#include "hash.h"
#include <vector>
#include <map>
namespace KG::Resource
{
	using MaterialSet = std::vector<KG::Utill::HashString>;
	struct MaterialMatch
	{
		MaterialSet defaultMaterial;
		std::map<KG::Utill::HashString, MaterialSet> materialMap;
		void SetDefaultMaterial(const MaterialSet& materials);
		void SetDefaultMaterial(MaterialSet&& materials);
		void AddMaterial(const KG::Utill::HashString& objectId, const MaterialSet& materials);
		void AddMaterial(const KG::Utill::HashString& objectId, MaterialSet&& materials);
		const MaterialSet& GetMaterial(const KG::Utill::HashString& objectId) const;
		void OnDrawGUI();
		void Clear();
	};
}