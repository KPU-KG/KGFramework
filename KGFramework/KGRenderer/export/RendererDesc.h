#pragma once
#include<Windows.h>
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "hash.h"
#include "tinyxml2.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS
namespace KG::Renderer
{
	struct DLL RenderTextureDesc
	{
		UINT width = 1920;
		UINT height = 1080;
		UINT length = 1;
		bool useCubeRender = false;
		bool useGSCubeRender = false;
		bool useGSArrayRender = false;
		bool useRenderTarget = true;
		bool useDeferredRender = true;
		bool useDepthStencilBuffer = true;
		bool uploadSRVRenderTarget = false;
		bool uploadSRVDepthBuffer = false;
		KG::Utill::HashString renderTargetTextureId = KG::Utill::HashString(0);
		KG::Utill::HashString depthBufferTextureId = KG::Utill::HashString(0);
	};


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

	struct RenderTextureProperty : public KG::Core::ISerializable
	{
	private:
		std::string name = "RenderTextureDesc";
		KG::Core::SerializableProperty<UINT> textureWidth;
		KG::Core::SerializableProperty<UINT> textureHeight;
		KG::Core::SerializableProperty<UINT> arrayCount;
		KG::Core::SerializableProperty<bool> useCubeRender;
		KG::Core::SerializableProperty<bool> useGSCubeRender;
		KG::Core::SerializableProperty<bool> useGSArrayRender;
		KG::Core::SerializableProperty<bool> useRenderTarget;
		KG::Core::SerializableProperty<bool> useDeferredRender;
		KG::Core::SerializableProperty<bool> useDepthStencilBuffer;
		KG::Core::SerializableProperty<bool> uploadSRVRenderTarget;
		KG::Core::SerializableProperty<bool> uploadSRVDepthBuffer;
		KG::Core::SerializableProperty<KG::Utill::HashString> renderTargetTextureId;
		KG::Core::SerializableProperty<KG::Utill::HashString> depthBufferTextureId;
	public:
		RenderTextureProperty(KG::Renderer::RenderTextureDesc& ref);
		virtual void SetName(const std::string& newName);
		virtual void OnDataLoad(tinyxml2::XMLElement* parentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;
	};


};
