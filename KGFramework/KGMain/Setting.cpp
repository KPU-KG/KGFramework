#include "Setting.h"
#include "Debug.h"
#include "tinyxml2.h"
#include <filesystem>
#include <fstream>
#include <Windows.h>

using namespace KG;

KG::Setting::Setting()
    :
    KG_PROP(clientWidth),
    KG_PROP(clientHeight),
    KG_PROP(fullScreenWidth),
    KG_PROP(fullScreenHeight),
    KG_PROP(fullScreen),
    KG_PROP(isVsync),
    KG_PROP(isEditMode),
    KG_PROP(isConsoleMode),
    KG_PROP(startScenePath)
{

}

KG::Setting::Setting(const Setting& other)
    : Setting()
{
    clientWidth = other.clientWidth;
    clientHeight = other.clientHeight;
    fullScreenWidth = other.fullScreenWidth;
    fullScreenHeight = other.fullScreenHeight;
    fullScreen = other.fullScreen;
    isVsync = other.isVsync;
    isEditMode = other.isEditMode;
    isConsoleMode = other.isConsoleMode;
    startScenePath = other.startScenePath;
}

Setting KG::Setting::Load()
{
	Setting data;
	if (std::filesystem::exists(Setting::fileDir))
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(Setting::fileDir.data());
		auto element = doc.FirstChildElement("Settings");
        data.clientWidthProp.OnDataLoad(element); 
        data.clientHeightProp.OnDataLoad(element);
        data.fullScreenWidthProp.OnDataLoad(element);
        data.fullScreenHeightProp.OnDataLoad(element);
        data.fullScreenProp.OnDataLoad(element);
        data.isVsyncProp.OnDataLoad(element);
        data.isEditModeProp.OnDataLoad(element);
        data.isConsoleModeProp.OnDataLoad(element);
        data.startScenePathProp.OnDataLoad(element);
    }
	return data;
}

void KG::Setting::Save(Setting& data)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLDeclaration* dec1 = doc.NewDeclaration();
	auto element = doc.NewElement("Settings");

    data.clientWidthProp.OnDataSave(element);
    data.clientHeightProp.OnDataSave(element);
    data.fullScreenWidthProp.OnDataSave(element);
    data.fullScreenHeightProp.OnDataSave(element);
    data.fullScreenProp.OnDataSave(element);
    data.isVsyncProp.OnDataSave(element);
    data.isEditModeProp.OnDataSave(element);
    data.isConsoleModeProp.OnDataSave(element);
    data.startScenePathProp.OnDataSave(element);

	doc.LinkEndChild(dec1);
	doc.LinkEndChild(element);
	doc.SaveFile(Setting::fileDir.data());
	DebugNormalMessage(L"세팅값을 저장하였습니다.");
}
