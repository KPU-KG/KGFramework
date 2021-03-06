#pragma once
#include <DirectXMath.h>
#include <string>
#include "tinyxml2.h"
#include "hash.h"
namespace KG::Utill::XMLConverter
{
	template<typename Ty>
	inline Ty XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		static_assert(true , "NotImplement");
	}

	template<typename Ty>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const Ty& ref)
	{
		static_assert(true , "NotImplement");
	}

#pragma region Def
	// int

	template<>
	inline int XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		return currentElement->IntAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const int& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value" , ref);
	}

	// float

	template<>
	inline float XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		return currentElement->FloatAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const float& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value" , ref);
	}

	// bool
	template<>
	inline bool XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		return currentElement->BoolAttribute("value");
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const bool& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value" , ref);
	}

	// XMFLOAT3
	template<>
	inline DirectX::XMFLOAT3 XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		DirectX::XMFLOAT3 result;
		result.x = currentElement->FloatAttribute("x");
		result.y = currentElement->FloatAttribute("y");
		result.z = currentElement->FloatAttribute("z");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const DirectX::XMFLOAT3& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("x" , ref.x);
		currentElement->SetAttribute("y" , ref.y);
		currentElement->SetAttribute("z" , ref.z);
	}

	// XMFLOAT4
	template<>
	inline DirectX::XMFLOAT4 XMLElementLoad(tinyxml2::XMLElement* objectElement , const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		DirectX::XMFLOAT4 result;
		result.x = currentElement->FloatAttribute("x");
		result.y = currentElement->FloatAttribute("y");
		result.z = currentElement->FloatAttribute("z");
		result.w = currentElement->FloatAttribute("w");
		return result;
	}
	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement , const std::string& title , const DirectX::XMFLOAT4& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("x" , ref.x);
		currentElement->SetAttribute("y" , ref.y);
		currentElement->SetAttribute("z" , ref.z);
		currentElement->SetAttribute("w" , ref.w);
	}


	//HashString
	template<>
	inline KG::Utill::HashString XMLElementLoad(tinyxml2::XMLElement* objectElement, const std::string& title)
	{
		auto* currentElement = objectElement->FirstChildElement(title.c_str());
		KG::Utill::HashString result;
		result = currentElement->UnsignedAttribute("value");
		return result;
	}

	template<>
	inline void XMLElementSave(tinyxml2::XMLElement* objectElement, const std::string& title, const KG::Utill::HashString& ref)
	{
		auto* currentElement = objectElement->InsertNewChildElement(title.c_str());
		currentElement->SetAttribute("value", ref.value);
	}


#pragma endregion
}
