#pragma once
#include <iostream>
#include <string>
#include <string_view>
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
		int GetGameResolutionWidth()
		{
			return fullScreen ? fullScreenWidth : clientWidth;
		};
		int GetGameResolutionHeigth()
		{
			return fullScreen ? fullScreenHeight : clientHeight;
		};
		static constexpr std::string_view fileDir = "setting.xml";
		static Setting Load();
		static void Save(const Setting& data);
	};
};