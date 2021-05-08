#include "Setting.h"
#include "Debug.h"
#include "tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <Windows.h>

using namespace KG;
Setting KG::Setting::Load()
{
	Setting data;
	if (std::filesystem::exists(Setting::fileDir))
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(Setting::fileDir.data());
		auto element = doc.FirstChildElement("Settings");

		{
			auto resolution = element->FirstChildElement("ScreenResolution");
			data.clientWidth = resolution->IntAttribute("width");
			data.clientHeight = resolution->IntAttribute("height");
		}

		{
			auto fullscreen = element->FirstChildElement("FullScreen");
			data.fullScreen = fullscreen->BoolAttribute("use");
		}

		{
			auto fullscreen = element->FirstChildElement("VSync");
			data.isVsync = fullscreen->BoolAttribute("use");
		}

	}
	return data;
}

void KG::Setting::Save(const Setting& data)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLDeclaration* dec1 = doc.NewDeclaration();
	auto element = doc.NewElement("Settings");

	{
		auto resolution = element->InsertNewChildElement("ScreenResolution");
		resolution->SetAttribute("width", data.clientWidth);
		resolution->SetAttribute("height", data.clientHeight);
	}

	{
		auto fullscreen = element->InsertNewChildElement("FullScreen");
		fullscreen->SetAttribute("use", data.fullScreen);
	}

	{
		auto fullscreen = element->InsertNewChildElement("VSync");
		fullscreen->SetAttribute("use", data.isVsync);
	}

	doc.LinkEndChild(dec1);
	doc.LinkEndChild(element);
	doc.SaveFile(Setting::fileDir.data());
	DebugNormalMessage(L"세팅값을 저장하였습니다.");
}
