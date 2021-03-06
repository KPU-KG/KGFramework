#pragma once
#include <string_view>
#include "tinyxml2.h"
#include "hash.h"
#include "ISerializable.h"
#include "XMLConverter.h"

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
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override
		{
			this->ref = KG::Utill::XMLConverter::XMLElementLoad<Ty>(objectElement, title);
		}

		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override
		{
			KG::Utill::XMLConverter::XMLElementSave<Ty>(objectElement, title, ref);
		}

		virtual void OnDrawGUI() override
		{
		}
	};
};
