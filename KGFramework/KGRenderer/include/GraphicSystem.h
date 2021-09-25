#pragma once
#include "ISystem.h"
#include "GraphicComponent.h"
namespace KG::System
{
	using namespace KG::Component;
	struct GeometrySystem : public IComponentSystem<GeometryComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IGeometryComponent>::name());
        // IComponentSystem을(를) 통해 상속됨
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

    struct Render2DSystem : public IComponentSystem<Render2DComponent>
    {
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IRender2DComponent>::name());
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnUpdate(float elapsedTime) override;
        virtual void OnPostUpdate(float elapsedTime) override;
        virtual void OnPreRender() override;
    };

    struct RenderSpriteSystem : public IComponentSystem<RenderSpriteComponent>
    {
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IRenderSpriteComponent>::name());
        // IComponentSystem을(를) 통해 상속됨
        virtual void OnUpdate(float elapsedTime) override;
        virtual void OnPostUpdate(float elapsedTime) override;
        virtual void OnPreRender() override;
    };

	struct Render3DSystem : public IComponentSystem<Render3DComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IRender3DComponent>::name());
        // IComponentSystem을(를) 통해 상속됨
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
        void OnProcessCulling(const DirectX::BoundingFrustum& frustum);
    };

	struct MaterialSystem : public IComponentSystem<MaterialComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IMaterialComponent>::name());
    protected:
		virtual void OnGetNewComponent(MaterialComponent* target) override;
	public:
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
	struct CameraSystem : public IComponentSystem<CameraComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<ICameraComponent>::name());
    protected:
		virtual void OnGetNewComponent( CameraComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct CubeCameraSystem : public IComponentSystem<CubeCameraComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<ICubeCameraComponent>::name());
    protected:
		virtual void OnGetNewComponent( CubeCameraComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};


	struct LightSystem : public IComponentSystem<LightComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<ILightComponent>::name());
    protected:
		virtual void OnGetNewComponent( LightComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};
	struct AvatarSystem : public IComponentSystem<BoneTransformComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IBoneTransformComponent>::name());
    public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct AnimationControllerSystem : public IComponentSystem<AnimationControllerComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IAnimationControllerComponent>::name());
    public:
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	struct ShadowCasterSystem : public IComponentSystem<ShadowCasterComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IShadowCasterComponent>::name());
    public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct ParticleEmitterSystem : public IComponentSystem<ParticleEmitterComponent>
	{
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<IParticleEmitterComponent>::name());
    public:
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

    struct PostProcessorSystem : public IComponentSystem<PostProcessManagerComponent>
    {
        KG::Utill::HashString systemComponentId = KG::Utill::HashString(KG::Component::ComponentID<PostProcessManagerComponent>::name());
    public:
        virtual void OnUpdate(float elapsedTime) override;
        virtual void OnPostUpdate(float elapsedTime) override;
        virtual void OnPreRender() override;
    };
}