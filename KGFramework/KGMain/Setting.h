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
		bool fullScreen = false;
		bool isVsync = false;
		static constexpr std::string_view fileDir = "setting.txt";
		static Setting Load();
		static void Save(const Setting& data);
	};
};