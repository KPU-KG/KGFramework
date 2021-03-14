#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <string>
#include "SerializableProperty.h"
#include "hash.h"
#include "ImguiHelper.h"
namespace KG::Utill::ImguiProperty
{
	constexpr static UINT elementalWidths[] = {100, 160, 200, 200};
	template <typename Ty>
	inline bool DrawGUIProperty(std::string& title, Ty& ref)
	{
		static_assert(true, "NotImplement");
	}

#pragma region DEF
	template <>
	inline bool DrawGUIProperty(std::string& title, UINT& ref)
	{
		ImGui::PushItemWidth(elementalWidths[0]);
		auto ret = ImGui::DragScalar(title.c_str(), ImGuiDataType_U32, &ref, 1.0f);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, int& ref)
	{
		ImGui::PushItemWidth(elementalWidths[0]);
		auto ret = ImGui::DragInt(title.c_str(), &ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, float& ref)
	{
		ImGui::PushItemWidth(elementalWidths[0]);
		auto ret = ImGui::DragFloat(title.c_str(), &ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, bool& ref)
	{
		ImGui::PushItemWidth(elementalWidths[0]);
		auto ret = ImGui::Checkbox(title.c_str(), &ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, DirectX::XMFLOAT2& ref)
	{
		ImGui::PushItemWidth(elementalWidths[1]);
		auto ret = ImGui::DragFloat2(title.c_str(), (float*)&ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, DirectX::XMFLOAT3& ref)
	{
		ImGui::PushItemWidth(elementalWidths[2]);
		auto ret = ImGui::DragFloat3(title.c_str(), (float*)&ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, DirectX::XMFLOAT4& ref)
	{
		ImGui::PushItemWidth(elementalWidths[3]);
		auto ret = ImGui::DragFloat4(title.c_str(), (float*)&ref);
		ImGui::PopItemWidth();
		return ret;
	}

	template <>
	inline bool DrawGUIProperty(std::string& title, KG::Utill::HashString& ref)
	{
		ImGui::PushItemWidth(elementalWidths[0]);
		auto ret = ImGui::InputHashString(title.c_str(), &ref);
		ImGui::PopItemWidth();
		return ret;
	}

#pragma endregion
}
