#pragma once
#include <string_view>
#include "tinyxml2.h"
#include "hash.h"
#include "XMLConverter.h"
#include "ImguiHelper.h"
namespace KG::Core
{
	class ISerializable
	{
	public:
		//���� ��ü�� �ڽ��� �̸� ���������� ���ο���, ��ü������ ���ǵ��� �ʾ����� (���� ��, ���� �� �� ) �ܺο��� ������Ʈ �˻�
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) = 0;

		//������ ���ο��� ������Ʈ ����
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) = 0;

		virtual bool OnDrawGUI() = 0;
		ImGuiContext* currentGUIContext = nullptr;
		bool DrawGUI(ImGuiContext* context)
		{
			currentGUIContext = context;
			ImGui::SetCurrentContext(context);
			return this->OnDrawGUI();
		}
	};
};
