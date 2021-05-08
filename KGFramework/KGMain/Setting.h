#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include "ISerializable.h"
#include "SerializableProperty.h"
namespace KG
{
	struct Setting
	{
		int clientWidth = 1600;
		int clientHeight = 900;
		int fullScreenWidth = 0;
		int fullScreenHeight = 0;
		bool fullScreen = false;
		bool isVsync = false;
        bool isEditMode = false;
        bool isConsoleMode = false;
        std::string startScenePath = "none";

        KG::Core::SerializableProperty<int> clientWidthProp;
        KG::Core::SerializableProperty<int> clientHeightProp;
        KG::Core::SerializableProperty<int> fullScreenWidthProp;
        KG::Core::SerializableProperty<int> fullScreenHeightProp;
        KG::Core::SerializableProperty<bool> fullScreenProp;
        KG::Core::SerializableProperty<bool> isVsyncProp;
        KG::Core::SerializableProperty<bool> isEditModeProp;
        KG::Core::SerializableProperty<bool> isConsoleModeProp;
        KG::Core::SerializableProperty<std::string> startScenePathProp;
        int GetGameResolutionWidth() const
		{
			return fullScreen ? fullScreenWidth : clientWidth;
		};
		int GetGameResolutionHeigth() const
		{
			return fullScreen ? fullScreenHeight : clientHeight;
		};
        Setting();
        Setting(const Setting& other);
		static inline std::string fileDir = "setting.xml";
		static Setting Load();
		static void Save(Setting& data);
	};
};