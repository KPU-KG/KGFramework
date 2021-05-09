#pragma once
#include "ISystem.h"
#include "FakeGraphicComponent.h"
namespace KG::System
{
    using namespace KG::Component;
    struct FakeGeometrySystem : public IComponentSystem<FakeGeometryComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeRender3DSystem : public IComponentSystem<FakeRender3DComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeMaterialSystem : public IComponentSystem<FakeMaterialComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeCameraSystem : public IComponentSystem<FakeCameraComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeCubeCameraSystem : public IComponentSystem<FakeCubeCameraComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeLightSystem : public IComponentSystem<FakeLightComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeAvatarSystem : public IComponentSystem<FakeBoneTransformComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeAnimationControllerSystem : public IComponentSystem<FakeAnimationControllerComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeShadowCasterSystem : public IComponentSystem<FakeShadowCasterComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
    struct FakeParticleEmitterSystem : public IComponentSystem<FakeParticleEmitterComponent>
    {
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPreRender() override;
    };
}