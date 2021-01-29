#pragma once
#include <d3d12.h>
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
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) {};
	};
};
