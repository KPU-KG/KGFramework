#pragma once
#include "ISystem.h"
#include "FakeGraphicComponent.h"
namespace KG::System
{
    using namespace KG::Component;
    struct FakeGeometrySystem : public IComponentSystem<FakeGeometryComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeRender3DSystem : public IComponentSystem<FakeRender3DComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeMaterialSystem : public IComponentSystem<FakeMaterialComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeCameraSystem : public IComponentSystem<FakeCameraComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeCubeCameraSystem : public IComponentSystem<FakeCubeCameraComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeLightSystem : public IComponentSystem<FakeLightComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeAvatarSystem : public IComponentSystem<FakeBoneTransformComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeAnimationControllerSystem : public IComponentSystem<FakeAnimationControllerComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeShadowCasterSystem : public IComponentSystem<FakeShadowCasterComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeParticleEmitterSystem : public IComponentSystem<FakeParticleEmitterComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeRender2DSystem : public IComponentSystem<FakeRender2DComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
    struct FakeRenderSpriteSystem : public IComponentSystem<FakeRenderSpriteComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };

    struct FakePostProcessorSystem : public IComponentSystem<FakePostProcessManagerComponent>
    {
        // IComponentSystem��(��) ���� ��ӵ�
        virtual void OnPreRender() override;
    };
}