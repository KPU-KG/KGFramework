#pragma once
#include <string_view>
#include <string>
#include <vector>
#include "tinyxml2.h"
#include "hash.h"
#include "ISerializable.h"
#include "XMLConverter.h"
#include "ImguiProperty.h"

#define CONST_KG_PROPERTY(name,ref) name(#name, ref, true)
#define KG_PROPERTY(name,ref) name(#name, ref)

namespace KG::Core
{
	template <typename Ty>
	struct SerializableProperty : public ISerializable
	{
		std::string title;
		Ty& ref;
		bool isConst = false;

		SerializableProperty(const std::string& title, Ty& ref, bool isConst = false)
			: ref(ref), title(title), isConst(isConst)
		{
		}

		auto& operator*()
		{
			return this->ref;
		}

		auto operator*() const
		{
			return this->ref;
		}
		auto operator->()
		{
			return &this->ref;
		}
		auto operator->() const
		{
			return &this->ref;
		}
		virtual void OnDataLoad(tinyxml2::XMLElement* parentElement) override
		{
			this->ref = KG::Utill::XMLConverter::XMLElementLoad<Ty>(parentElement, title);
		}

		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override
		{
			KG::Utill::XMLConverter::XMLElementSave<Ty>(parentElement, title, ref);
		}

		virtual bool OnDrawGUI() override
		{
			return KG::Utill::ImguiProperty::DrawGUIProperty<Ty>(this->title, this->ref);
		}
	};

	template <typename Ty>
	struct SerializableEnumProperty : public ISerializable
	{
		std::string title;
		Ty& ref;
		int currentIndex = 0;
		bool isConst = false;
		using EnumMap = std::vector<std::pair<Ty, std::string>>;
		EnumMap map;
		SerializableEnumProperty(const std::string& title, Ty& ref, const std::vector<std::pair<Ty, std::string>>& map, bool isConst = false)
			: ref(ref), title(title), isConst(isConst), map(map)
		{
		}

		auto& operator*()
		{
			return this->ref;
		}

		auto operator*() const
		{
			return this->ref;
		}
		auto operator->()
		{
			return &this->ref;
		}
		auto operator->() const
		{
			return &this->ref;
		}
		virtual void OnDataLoad(tinyxml2::XMLElement* parentElement) override
		{
			this->ref = (Ty)KG::Utill::XMLConverter::XMLElementLoad<int>(parentElement, title);
		}

		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override
		{
			int r = (int)this->ref;
			KG::Utill::XMLConverter::XMLElementSave<int>(parentElement, title, r);
		}
		static bool ItemGetter(void* data, int n, const char** out_str)
		{
			EnumMap& inMap = *static_cast<EnumMap*>(data);
			*out_str = inMap[n].second.c_str();
			return true;
		}
		virtual bool OnDrawGUI() override
		{
			bool ret = false;
			if ( ret = ImGui::Combo(title.c_str(), &this->currentIndex, &SerializableEnumProperty<Ty>::ItemGetter, &map, map.size()) )
			{
				ref = (Ty)this->map[currentIndex].first;
			}
			return ret;
		}
	};
};
