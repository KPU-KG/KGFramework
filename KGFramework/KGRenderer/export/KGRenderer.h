#pragma once
#define NOMINMAX
#include <Windows.h>
#include "ISystem.h"
#include "IGraphicComponent.h"
#include "ObjectContainer.h"
#include "RendererDesc.h"
#include "MaterialMatch.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS

namespace KG::Renderer
{
    class DLL IKGRenderer
    {
    protected:
        RendererDesc desc;
        RendererSetting setting;
        KG::Utill::HashString skymapTextureId = KG::Utill::HashString(0);
        IKGRenderer() = default;
    public:
        virtual ~IKGRenderer() = default;
        void SetDesc(const RendererDesc& desc);
        void SetSetting(const RendererSetting& setting);

        virtual void Initialize(const RendererDesc& desc, const RendererSetting& setting)
        {
            this->SetDesc(desc);
            this->SetSetting(setting);
            this->Initialize();
        }
        virtual void Initialize() = 0;
        virtual void Render() = 0;
        virtual void PreRenderEditorUI() = 0;
        virtual void* GetImGUIContext() = 0;
        virtual void Update(float elaspedTime) = 0;
        virtual void SetGameTime(double gameTime) = 0;
        virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) = 0;
        virtual void PreloadModels(std::vector<KG::Utill::HashString>&& ids) = 0;
        virtual void SetEditUIRender(bool isRender) = 0;
        virtual UINT QueryMaterialIndex(const KG::Utill::HashString& materialId) const = 0;
        virtual void DebugUIRender() {};

        virtual double GetGameTime() const = 0;

        virtual void SetSkymapTextureId(const KG::Utill::HashString& id)
        {
            this->skymapTextureId = id;
        }
        virtual KG::Utill::HashString GetSkymapTexutreId() const
        {
            return this->skymapTextureId;
        }

        virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;

        virtual KG::Component::IRender3DComponent* GetNewRenderComponent() = 0;
        virtual KG::Component::IRender2DComponent* GetNewRender2DComponent() = 0;
        virtual KG::Component::IRenderSpriteComponent* GetNewRenderSpriteComponent() = 0;
        virtual KG::Component::IGeometryComponent* GetNewGeomteryComponent() = 0;
        virtual KG::Component::IMaterialComponent* GetNewMaterialComponent() = 0;
        virtual KG::Component::ICameraComponent* GetNewCameraComponent() = 0;
        virtual KG::Component::ICubeCameraComponent* GetNewCubeCameraComponent() = 0;
        virtual KG::Component::ILightComponent* GetNewLightComponent() = 0;
        virtual KG::Component::IBoneTransformComponent* GetNewBoneTransformComponent() = 0;
        virtual KG::Component::IShadowCasterComponent* GetNewShadowCasterComponent() = 0;
        virtual KG::Component::IAnimationControllerComponent* GetNewAnimationControllerComponent() = 0;
        virtual KG::Component::IParticleEmitterComponent* GetNewParticleEmitterComponent() = 0;
        virtual KG::Component::IPostProcessManagerComponent* GetNewPostProcessorComponent() = 0;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials) = 0;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials) = 0;
    };
    DLL KG::Renderer::IKGRenderer* GetD3D12Renderer();
    DLL KG::Renderer::IKGRenderer* GetFakeRenderer();
}
