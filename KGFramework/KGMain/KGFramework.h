#pragma once
#include <Windows.h>
#include "Setting.h"
#include "GameTimer.h"
namespace KG
{
	struct EngineDesc
	{
		HWND hWnd;
		HINSTANCE hInst;
	};

	class GameFramework
	{
		EngineDesc engineDesc;
		Setting setting;
		GameTimer timer;

	public:
		bool Initialize(const EngineDesc& engineDesc, const Setting& setting);
		void OnProcess();
		void OnClose();

	private:
		std::wstring windowText = L"KG Framework : FPS : ##########";
		void UpdateWindowText();
	};
};