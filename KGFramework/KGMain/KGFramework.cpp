#include "KGFramework.h"
#include "KGRenderer.h"
#include "Systems.h"

KG::GameFramework::GameFramework()
{
}
KG::GameFramework::~GameFramework()
{
}
KG::GameFramework::GameFramework(const GameFramework& rhs)
{
}
KG::GameFramework& KG::GameFramework::operator=(const GameFramework& rhs)
{
	return *this;
}
KG::GameFramework::GameFramework(GameFramework&& rhs)
{
}
KG::GameFramework& KG::GameFramework::operator=(GameFramework&& rhs)
{
	return *this;
}
bool KG::GameFramework::Initialize(const EngineDesc& engineDesc, const Setting& setting)
{
	//Desc류 세팅
	this->engineDesc = engineDesc;
	this->setting = setting;

	//Renderer
	this->renderer = std::unique_ptr<KG::Renderer::IKGRenderer>(KG::Renderer::GetD3D12Renderer());

	KG::Renderer::RendererDesc renderDesc;
	renderDesc.hInst = this->engineDesc.hInst;
	renderDesc.hWnd = this->engineDesc.hWnd;

	KG::Renderer::RendererSetting renderSetting;
	renderSetting.clientWidth = this->setting.clientWidth;
	renderSetting.clientHeight = this->setting.clientHeight;
	renderSetting.isVsync = this->setting.isVsync;

	this->renderer->Initialize(renderDesc, renderSetting);

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
