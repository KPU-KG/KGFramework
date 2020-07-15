#pragma once
#include "IComponent.h"
namespace KG::Component
{
	class IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};
	REGISTER_COMPONENT_ID(IRenderComponent);

	class MeshRenderer : public IRenderComponent
	{
	};

	REGISTER_COMPONENT_ID(MeshRenderer);

	class LightRenderer : public IRenderComponent
	{

	};
	REGISTER_COMPONENT_ID(LightRenderer);

}