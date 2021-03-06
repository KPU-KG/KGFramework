#include "pch.h"
#include "KGRenderer.h"
#include "KGDXRenderer.h"
using namespace KG::Renderer;

void IKGRenderer::SetDesc(const RendererDesc& desc)
{
	this->desc = desc;
}

void IKGRenderer::SetSetting(const RendererSetting& setting)
{
	auto prev = this->setting;
	this->setting = setting;
	OnChangeSettings(prev, this->setting);
}

DLL KG::Renderer::IKGRenderer* KG::Renderer::GetD3D12Renderer()
{
	return new KGDXRenderer();
}