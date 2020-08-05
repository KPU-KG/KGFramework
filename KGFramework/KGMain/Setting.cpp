#include "Setting.h"
#include "Debug.h"
#include <filesystem>
#include <fstream>
#include <Windows.h>

using namespace KG;
Setting KG::Setting::Load()
{
	Setting data;
	if (std::filesystem::exists(Setting::fileDir))
	{
		std::ifstream in(Setting::fileDir.data());
		in >> data.clientWidth;
		in >> data.clientHeight;
		in >> std::boolalpha >> data.fullScreen;
		in >> std::boolalpha >> data.isVsync;
	}
	return data;
}

void KG::Setting::Save(const Setting& data)
{
	std::ofstream out(Setting::fileDir.data(), std::ios::out | std::ios::trunc);
	out << data.clientWidth << std::endl;
	out << data.clientHeight << std::endl;
	out << std::boolalpha << data.fullScreen << std::endl;
	out << std::boolalpha << data.isVsync << std::endl;
	DebugNormalMessage(L"세팅값을 저장하였습니다.");
}
