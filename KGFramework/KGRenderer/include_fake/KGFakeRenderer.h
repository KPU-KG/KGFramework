#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGRenderer.h"
#include "FakeGraphicComponent.h"
#include "FakeSystems.h"
namespace KG::Renderer
{
    struct FakeGraphicSystems
    {
        KG::System::FakeRender3DSystem render3DSystem;
        KG::System::FakeRender2DSystem render2DSystem;
        KG::System::FakeRenderSpriteSystem renderSpriteSystem;
        KG::System::FakeGeometrySystem geometrySystem;
        KG::System::FakeMaterialSystem materialSystem;
        KG::System::FakeCameraSystem cameraSystem;
        KG::System::FakeCubeCameraSystem cubeCameraSystem;
        KG::System::FakeLightSystem lightSystem;
        KG::System::FakeAvatarSystem avatarSystem;
        KG::System::FakeAnimationControllerSystem animationControllerSystem;
        KG::System::FakeShadowCasterSystem shadowSystem;
        KG::System::FakeParticleEmitterSystem particleSystem;
        KG::System::FakePostProcessorSystem postProcessSystem;

        void OnPreRender()
        {
            this->geometrySystem.OnPreRender();
            this->materialSystem.OnPreRender();
            this->render2DSystem.OnPreRender();
            this->render3DSystem.OnPreRender();
            this->renderSpriteSystem.OnPreRender();
            this->cameraSystem.OnPreRender();
            this->cubeCameraSystem.OnPreRender();
            this->shadowSystem.OnPreRender();
            this->lightSystem.OnPreRender();
            this->avatarSystem.OnPreRender();
            this->animationControllerSystem.OnPreRender();
            this->particleSystem.OnPreRender();
            this->postProcessSystem.OnPreRender();
        }

        void OnUpdate(float elapsedTime)
        {
            this->geometrySystem.OnUpdate(elapsedTime);
            this->materialSystem.OnUpdate(elapsedTime);
            this->render2DSystem.OnUpdate(elapsedTime);
            this->render3DSystem.OnUpdate(elapsedTime);
            this->renderSpriteSystem.OnUpdate(elapsedTime);
            this->cameraSystem.OnUpdate(elapsedTime);
            this->cubeCameraSystem.OnUpdate(elapsedTime);
            this->lightSystem.OnUpdate(elapsedTime);
            this->avatarSystem.OnUpdate(elapsedTime);
            this->animationControllerSystem.OnUpdate(elapsedTime);
            this->shadowSystem.OnUpdate(elapsedTime);
            this->particleSystem.OnUpdate(elapsedTime);
            this->postProcessSystem.OnUpdate(elapsedTime);
        }
        void OnPostUpdate(float elapsedTime)
        {
            this->geometrySystem.OnPostUpdate(elapsedTime);
            this->materialSystem.OnPostUpdate(elapsedTime);
            this->render2DSystem.OnPostUpdate(elapsedTime);
            this->render3DSystem.OnPostUpdate(elapsedTime);
            this->renderSpriteSystem.OnPostUpdate(elapsedTime);
            this->cameraSystem.OnPostUpdate(elapsedTime);
            this->cubeCameraSystem.OnPostUpdate(elapsedTime);
            this->lightSystem.OnPostUpdate(elapsedTime);
            this->avatarSystem.OnPostUpdate(elapsedTime);
            this->animationControllerSystem.OnPostUpdate(elapsedTime);
            this->shadowSystem.OnPostUpdate(elapsedTime);
            this->particleSystem.OnPostUpdate(elapsedTime);
            this->postProcessSystem.OnPostUpdate(elapsedTime);
        }

        void PostComponentProvider(KG::Component::ComponentProvider& provider)
        {
            this->geometrySystem.OnPostProvider(provider);
            this->materialSystem.OnPostProvider(provider);
            this->render2DSystem.OnPostProvider(provider);
            this->render3DSystem.OnPostProvider(provider);
            this->renderSpriteSystem.OnPostProvider(provider);
            this->cameraSystem.OnPostProvider(provider);
            this->cubeCameraSystem.OnPostProvider(provider);
            this->lightSystem.OnPostProvider(provider);
            this->avatarSystem.OnPostProvider(provider);
            this->animationControllerSystem.OnPostProvider(provider);
            this->shadowSystem.OnPostProvider(provider);
            this->particleSystem.OnPostProvider(provider);
            this->postProcessSystem.OnPostProvider(provider);
        }

        void Clear()
        {
            this->geometrySystem.Clear();
            this->materialSystem.Clear();
            this->render2DSystem.Clear();
            this->render3DSystem.Clear();
            this->renderSpriteSystem.Clear();
            this->cameraSystem.Clear();
            this->cubeCameraSystem.Clear();
            this->lightSystem.Clear();
            this->avatarSystem.Clear();
            this->animationControllerSystem.Clear();
            this->shadowSystem.Clear();
            this->particleSystem.Clear();
            this->postProcessSystem.Clear();
        }
    };


    class KGFakeRenderer : public IKGRenderer
    {
    public:
        std::unique_ptr<FakeGraphicSystems> graphicSystems = nullptr;

        static KGFakeRenderer* GetInstance();
        static inline KGFakeRenderer* instance = nullptr;


        // IKGRenderer??(??) ???? ??????
        virtual void Initialize() override;
        virtual void Render() override;
        virtual void PreRenderEditorUI() override;
        virtual void* GetImGUIContext() override;
        virtual void Update(float elaspedTime) override;
        virtual void SetGameTime(double gameTime) override;
        virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;
        virtual void PreloadModels(std::vector<KG::Utill::HashString>&& ids) override;
        virtual void SetEditUIRender(bool isRender) override;
        virtual UINT QueryMaterialIndex(const KG::Utill::HashString& materialId) const override;
        virtual double GetGameTime() const override;
        virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

        virtual KG::Component::IRender3DComponent* GetNewRenderComponent() override;
        virtual KG::Component::IRender2DComponent* GetNewRender2DComponent() override;
        virtual KG::Component::IRenderSpriteComponent* GetNewRenderSpriteComponent() override;
        virtual KG::Component::IGeometryComponent* GetNewGeomteryComponent() override;
        virtual KG::Component::IMaterialComponent* GetNewMaterialComponent() override;
        virtual KG::Component::ICameraComponent* GetNewCameraComponent() override;
        virtual KG::Component::ICubeCameraComponent* GetNewCubeCameraComponent() override;
        virtual KG::Component::ILightComponent* GetNewLightComponent() override;
        virtual KG::Component::IBoneTransformComponent* GetNewBoneTransformComponent() override;
        virtual KG::Component::IShadowCasterComponent* GetNewShadowCasterComponent() override;
        virtual KG::Component::IAnimationControllerComponent* GetNewAnimationControllerComponent() override;
        virtual KG::Component::IParticleEmitterComponent* GetNewParticleEmitterComponent() override;
        virtual KG::Component::IPostProcessManagerComponent* GetNewPostProcessorComponent() override;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials) override;
        virtual KG::Core::GameObject* LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials) override;

    };
}