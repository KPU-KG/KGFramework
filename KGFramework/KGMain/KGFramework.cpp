#include "KGFramework.h"

bool KG::GameFramework::Initialize(const EngineDesc& engineDesc, const Setting& setting)
{
	//Desc류 세팅
	this->engineDesc = engineDesc;
	this->setting = setting;


	//자원 미리 할당
	this->windowText.reserve(100);

	return true;
}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();
}

void KG::GameFramework::OnClose()
{
	this->setting.Save(this->setting);
}

void KG::GameFramework::UpdateWindowText()
{
	static float duration = 0.0f;
	duration += this->timer.GetTimeElapsed();
	if (duration >= 1.0f)
	{
		auto frame = this->timer.GetFrameRate();
		::_itow_s(frame, this->windowText.data() + 21, 10, 10);
		::SetWindowText(this->engineDesc.hWnd, this->windowText.data());
		duration = 0.0f;
	}
}
