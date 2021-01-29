#pragma once
#define NOMINMAX
#include <Windows.h>
#include "ISystem.h"
#include "GraphicComponent.h"
#include "ObjectContainer.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS

namespace KG::Resource
{
	using MaterialSet = std::vector<KG::Utill::HashString>;
	struct DLL MaterialMatch
	{
		MaterialSet defaultMaterial;
		std::map<KG::Utill::HashString, MaterialSet> materialMap;
		void SetDefaultMaterial( const MaterialSet& materials );
		void SetDefaultMaterial( MaterialSet&& materials );
		void AddMaterial( const KG::Utill::HashString& objectId, const MaterialSet& materials );
		void AddMaterial( const KG::Utill::HashString& objectId, MaterialSet&& materials );
		const MaterialSet& GetMaterial( const KG::Utill::HashString& objectId ) const;
	};
}

namespace KG::Renderer
{
	struct DLL RenderTextureDesc
	{
		size_t width;
		size_t height;
		bool useCubeRender = false;
		bool useRenderTarget = true;
		bool useDeferredRender = true;
		bool useDepthStencilBuffer = true;
		bool uploadSRVRenderTarget = false;
		bool uploadSRVDepthBuffer = false;
		KG::Utill::HashString renderTargetTextureId = KG::Utill::HashString( 0 );
		KG::Utill::HashString depthBufferTextureId = KG::Utill::HashString( 0 );
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

	class DLL IKGRenderer
	{
	protected:
		RendererDesc desc;
		RendererSetting setting;
		KG::Utill::HashString skymapTextureId = KG::Utill::HashString(0);
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
		virtual void Update(float elaspedTime) = 0;
		virtual void OnChangeSettings( const RendererSetting& prev, const RendererSetting& next ) = 0;

		virtual void SetSkymapTextureId( const KG::Utill::HashString& id )
		{
			this->skymapTextureId = id;
		}
		virtual KG::Utill::HashString GetSkymapTexutreId() const
		{
			return this->skymapTextureId;
		}

		virtual KG::Component::Render3DComponent* GetNewRenderComponent() = 0;
		virtual KG::Component::GeometryComponent* GetNewGeomteryComponent() = 0;
		virtual KG::Component::GeometryComponent* GetNewGeomteryComponent( const KG::Utill::HashString& id, UINT index = 0) = 0;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponent() = 0;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponent( const KG::Utill::HashString& id ) = 0;
		virtual KG::Component::MaterialComponent* GetNewMaterialComponentFromShader( const KG::Utill::HashString& id ) = 0;
		virtual KG::Component::CameraComponent* GetNewCameraComponent() = 0;
		virtual KG::Component::CubeCameraComponent* GetNewCubeCameraComponent() = 0;
		virtual KG::Component::LightComponent* GetNewLightComponent() = 0;
		virtual KG::Component::BoneTransformComponent* GetNewBoneTransformComponent() = 0;
		// virtual KG::Component::AnimationStreamerComponent* GetNewBoneAnimationStreamComponent( const KG::Utill::HashString& id, UINT index = 0 ) = 0;
		virtual KG::Component::AnimationContollerComponent* GetNewAnimationContollerComponent() = 0;
		virtual KG::Core::GameObject* LoadFromModel( const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials ) = 0;

	};
	DLL KG::Renderer::IKGRenderer* GetD3D12Renderer();
}

