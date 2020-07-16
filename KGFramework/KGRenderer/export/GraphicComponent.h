#pragma once
#include "IComponent.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS


namespace KG::Component
{
	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};
	REGISTER_COMPONENT_ID(IRenderComponent);

	class DLL MeshRenderer : public IRenderComponent
	{
	};

	REGISTER_COMPONENT_ID(MeshRenderer);

	class DLL LightRenderer : public IRenderComponent
	{

	};
	REGISTER_COMPONENT_ID(LightRenderer);

}