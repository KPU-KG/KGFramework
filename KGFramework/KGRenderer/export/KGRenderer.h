#pragma once
#include <Windows.h>

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS


namespace KG::Renderer
{
	struct DLL RendererDesc
	{
		HWND hWnd;
		HINSTANCE hInst;
	};
	struct DLL RendererSetting
	{
		int clientWidth;
		int clientHeight;
		size_t maxSwapChainCount = 2;
		size_t maxFrameResources = 3;
		bool isVsync = false;
	};

	class DLL IKGRenderer
	{
	protected:
		RendererDesc desc;
		RendererSetting setting;
		IKGRenderer() = default;
	public:
		virtual ~IKGRenderer() = default;
		void SetDesc(const RendererDesc& desc);
		void SetSetting(const RendererSetting& setting);

		virtual void Initialize() = 0;
		virtual void Render() = 0;

		virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) = 0;
	};
}