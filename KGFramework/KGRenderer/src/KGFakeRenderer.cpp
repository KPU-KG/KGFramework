#include "KGFakeRenderer.h"
#include "FakeGraphicComponent.h"
#include "FakeSystems.h"
#include "ResourceContainer.h"
#include "..\include_fake\KGFakeRenderer.h"

KG::Renderer::KGFakeRenderer* KG::Renderer::KGFakeRenderer::GetInstance()
{
    return KGFakeRenderer::instance;
}

void KG::Renderer::KGFakeRenderer::Initialize()
{
    KGFakeRenderer::instance = this;
    this->graphicSystems = std::make_unique<FakeGraphicSystems>();
}

void KG::Renderer::KGFakeRenderer::Render()
{
}

void KG::Renderer::KGFakeRenderer::PreRenderEditorUI()
{
}

void* KG::Renderer::KGFakeRenderer::GetImGUIContext()
{
    return nullptr;
}

void KG::Renderer::KGFakeRenderer::Update(float elaspedTime)
{
}

void KG::Renderer::KGFakeRenderer::SetGameTime(double gameTime)
{
}

void KG::Renderer::KGFakeRenderer::OnChangeSettings(const RendererSetting& prev, const RendererSetting& next)
{
}

void KG::Renderer::KGFakeRenderer::PreloadModels(std::vector<KG::Utill::HashString>&& ids)
{
    KG::Resource::FakeResourceContainer::GetInstance()->PreLoadModels(std::move(ids));
}

void KG::Renderer::KGFakeRenderer::SetEditUIRender(bool isRender)
{
}

UINT KG::Renderer::KGFakeRenderer::QueryMaterialIndex(const KG::Utill::HashString& materialId) const
{
    return 0;
}

double KG::Renderer::KGFakeRenderer::GetGameTime() const
{
    return 0.0;
}

void KG::Renderer::KGFakeRenderer::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
    this->graphicSystems->PostComponentProvider(provider);
}

KG::Component::IRender3DComponent* KG::Renderer::KGFakeRenderer::GetNewRenderComponent()
{
    return static_cast<KG::Component::IRender3DComponent*>(this->graphicSystems->render3DSystem.GetNewComponent());
}

KG::Component::IRender2DComponent* KG::Renderer::KGFakeRenderer::GetNewRender2DComponent()
{
    return static_cast<KG::Component::IRender2DComponent*>(this->graphicSystems->render2DSystem.GetNewComponent());
}

KG::Component::IRenderSpriteComponent* KG::Renderer::KGFakeRenderer::GetNewRenderSpriteComponent()
{
    return static_cast<KG::Component::IRenderSpriteComponent*>(this->graphicSystems->renderSpriteSystem.GetNewComponent());
}

KG::Component::IGeometryComponent* KG::Renderer::KGFakeRenderer::GetNewGeomteryComponent()
{
    return static_cast<KG::Component::IGeometryComponent*>(this->graphicSystems->geometrySystem.GetNewComponent());
}

KG::Component::IMaterialComponent* KG::Renderer::KGFakeRenderer::GetNewMaterialComponent()
{
    return static_cast<KG::Component::IMaterialComponent*>(this->graphicSystems->materialSystem.GetNewComponent());
}

KG::Component::ICameraComponent* KG::Renderer::KGFakeRenderer::GetNewCameraComponent()
{
    return static_cast<KG::Component::ICameraComponent*>(this->graphicSystems->cameraSystem.GetNewComponent());
}

KG::Component::ICubeCameraComponent* KG::Renderer::KGFakeRenderer::GetNewCubeCameraComponent()
{
    return static_cast<KG::Component::ICubeCameraComponent*>(this->graphicSystems->cubeCameraSystem.GetNewComponent());
}

KG::Component::ILightComponent* KG::Renderer::KGFakeRenderer::GetNewLightComponent()
{
    return static_cast<KG::Component::ILightComponent*>(this->graphicSystems->lightSystem.GetNewComponent());
}

KG::Component::IShadowCasterComponent* KG::Renderer::KGFakeRenderer::GetNewShadowCasterComponent()
{
    return static_cast<KG::Component::IShadowCasterComponent*>(this->graphicSystems->shadowSystem.GetNewComponent());
}

KG::Component::IBoneTransformComponent* KG::Renderer::KGFakeRenderer::GetNewBoneTransformComponent()
{
    return static_cast<KG::Component::IBoneTransformComponent*>(this->graphicSystems->avatarSystem.GetNewComponent());
}

KG::Component::IAnimationControllerComponent* KG::Renderer::KGFakeRenderer::GetNewAnimationControllerComponent()
{
    auto* anim = static_cast<KG::Component::IAnimationControllerComponent*>(this->graphicSystems->animationControllerSystem.GetNewComponent());
    return anim;
}

KG::Component::IParticleEmitterComponent* KG::Renderer::KGFakeRenderer::GetNewParticleEmitterComponent()
{
    auto* particleComp = static_cast<KG::Component::IParticleEmitterComponent*>(this->graphicSystems->particleSystem.GetNewComponent());
    return particleComp;
}

KG::Component::IPostProcessManagerComponent* KG::Renderer::KGFakeRenderer::GetNewPostProcessorComponent()
{
    auto* anim = static_cast<KG::Component::IPostProcessManagerComponent*>(this->graphicSystems->postProcessSystem.GetNewComponent());
    return anim;
}

KG::Core::GameObject* KG::Renderer::KGFakeRenderer::LoadFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const KG::Resource::MaterialMatch& materials)
{
    return KG::Resource::FakeResourceContainer::GetInstance()->CreateObjectFromModel(id, container, materials);
}

KG::Core::GameObject* KG::Renderer::KGFakeRenderer::LoadFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& materials)
{
    return KG::Resource::FakeResourceContainer::GetInstance()->CreateObjectFromModel(id, scene, materials);
}

