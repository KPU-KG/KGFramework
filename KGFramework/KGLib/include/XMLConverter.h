#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include <tuple>
#include "tinyxml2.h"
#include "hash.h"
#include "KGColor.h"
namespace KG::Utill::XMLConverter
{
	template<typename Ty>
	inline Ty XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		static_assert(true, "NotImplement");
	}

	template<typename Ty>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const Ty& ref)
	{
		static_assert(true, "NotImplement");
	}

#pragma region Def
	// uint
	template<>
	inline UINT XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		return currentElement->IntAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const UINT& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value", ref);
	}

	// int

	template<>
	inline int XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		return currentElement->IntAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const int& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value", ref);
	}

	// float

	template<>
	inline float XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		return currentElement->FloatAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const float& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value", ref);
	}

	// bool
	template<>
	inline bool XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		return currentElement->BoolAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const bool& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value", ref);
	}


	// XMFLOAT2
	template<>
	inline DirectX::XMFLOAT2 XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		DirectX::XMFLOAT2 result;
		result.x = currentElement->FloatAttribute("x");
		result.y = currentElement->FloatAttribute("y");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const DirectX::XMFLOAT2& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("x", ref.x);
		currentElement->SetAttribute("y", ref.y);
	}


	// KG::Utill::Color
	template<>
	inline KG::Utill::Color XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		KG::Utill::Color result;
		result.r = currentElement->FloatAttribute("r");
		result.g = currentElement->FloatAttribute("g");
		result.b = currentElement->FloatAttribute("b");
		result.a = currentElement->FloatAttribute("a");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const KG::Utill::Color& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("r", ref.r);
		currentElement->SetAttribute("g", ref.g);
		currentElement->SetAttribute("b", ref.b);
		currentElement->SetAttribute("a", ref.a);
	}

	// XMFLOAT3
	template<>
	inline DirectX::XMFLOAT3 XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		DirectX::XMFLOAT3 result;
		result.x = currentElement->FloatAttribute("x");
		result.y = currentElement->FloatAttribute("y");
		result.z = currentElement->FloatAttribute("z");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const DirectX::XMFLOAT3& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("x", ref.x);
		currentElement->SetAttribute("y", ref.y);
		currentElement->SetAttribute("z", ref.z);
	}

	// XMFLOAT4
	template<>
	inline DirectX::XMFLOAT4 XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		DirectX::XMFLOAT4 result;
		result.x = currentElement->FloatAttribute("x");
		result.y = currentElement->FloatAttribute("y");
		result.z = currentElement->FloatAttribute("z");
		result.w = currentElement->FloatAttribute("w");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const DirectX::XMFLOAT4& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("x", ref.x);
		currentElement->SetAttribute("y", ref.y);
		currentElement->SetAttribute("z", ref.z);
		currentElement->SetAttribute("w", ref.w);
	}


	//HashString
	template<>
	inline KG::Utill::HashString XMLElementLoad(tinyxml2::XMLElement* parentElement, const std::string& title)
	{
		auto* currentElement = parentElement->FirstChildElement(title.c_str());
		KG::Utill::HashString result;
		result = KG::Utill::HashString(currentElement->Attribute("srcString"));
		auto value = currentElement->UnsignedAttribute("hash_id");
		return  result.value != value ? KG::Utill::HashString(value) : result;
	}

	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* parentElement, const std::string& title, const KG::Utill::HashString& ref)
	{
		auto* currentElement = parentElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("hash_id", ref.value);
		currentElement->SetAttribute("srcString", ref.srcString.c_str());
	}

#pragma endregion
}
