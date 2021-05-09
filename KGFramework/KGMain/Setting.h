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
        bool isPxDebugger = false;
        bool isEditMode = false;
        bool isConsoleMode = false;
        bool isStartServer = false;
        bool isStartClient = false;
        bool isStartLogin = false;
        std::string ipAddress = "127.0.0.1";
        std::string startScenePath = "none";

        KG::Core::SerializableProperty<int> clientWidthProp;
        KG::Core::SerializableProperty<int> clientHeightProp;
        KG::Core::SerializableProperty<int> fullScreenWidthProp;
        KG::Core::SerializableProperty<int> fullScreenHeightProp;
        KG::Core::SerializableProperty<bool> fullScreenProp;
        KG::Core::SerializableProperty<bool> isVsyncProp;
        KG::Core::SerializableProperty<bool> isPxDebuggerProp;
        KG::Core::SerializableProperty<bool> isEditModeProp;
        KG::Core::SerializableProperty<bool> isConsoleModeProp;
        KG::Core::SerializableProperty<bool> isStartServerProp;
        KG::Core::SerializableProperty<bool> isStartClientProp;
        KG::Core::SerializableProperty<bool> isStartLoginProp;
        KG::Core::SerializableProperty<std::string> ipAddressProp;
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
        void DrawGui();
		static inline std::string fileDir = "setting.xml";
		static Setting Load();
		static void Save(Setting& data);
	};
};