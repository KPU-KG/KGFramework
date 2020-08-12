#pragma once
#define NOMINMAX
#include <Windows.h>
#include "ISystem.h"
#include "GraphicComponent.h"

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
		bool msaa4xEnable = false;
		UINT msaa4xQualityLevel = 0;
	};

	class DLL IKGRenderer
	{
	protected:
		RendererDesc desc;
		RendererSetting setting;
		IKGRenderer() = default;
	public:
		virtual ~IKGRenderer() = default;
		void SetDesc( const RendererDesc& desc );
		void SetSetting( const RendererSetting& setting );

		virtual void Initialize( const RendererDesc& desc, const RendererSetting& setting )
		{
			this->SetDesc( desc );
			this->SetSetting( setting );
			this->Initialize();
		}
		virtual void Initialize() = 0;
		virtual void Render() = 0;
		virtual void Update() = 0;
		virtual void OnChangeSettings( const RendererSetting& prev, const RendererSetting& next ) = 0;

		virtual KG::Component::Render3DComponent* GetNewRenderComponent() = 0;
		virtual KG::Component::GeometryComponent* GetNewGeomteryComponent( const KG::Utill::HashString& id ) = 0;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponent( const KG::Utill::HashString& id ) = 0;
		virtual KG::Component::CameraComponent* GetNewCameraComponent() = 0;
		virtual KG::Component::LightComponent* GetNewLightComponent() = 0;

	};
	DLL KG::Renderer::IKGRenderer* GetD3D12Renderer();
}

