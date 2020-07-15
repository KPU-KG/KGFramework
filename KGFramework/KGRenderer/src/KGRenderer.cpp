#include "KGRenderer.h"
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
