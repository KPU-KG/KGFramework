#pragma once
#include "ISystem.h"
#include "GraphicComponent.h"
namespace KG::System
{
	using namespace KG::Component;
	struct GeometrySystem : public IComponentSystem<GeometryComponent>
	{
		// IComponentSystem을(를) 통해 상속됨
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct Render3DSystem : public IComponentSystem<Render3DComponent>
	{
		// IComponentSystem을(를) 통해 상속됨
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	struct MaterialSystem : public IComponentSystem<MaterialComponent>
	{
	protected:
		virtual void OnGetNewComponent(MaterialComponent* target) override;
	public:
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
	struct CameraSystem : public IComponentSystem<CameraComponent>
	{
	protected:
		virtual void OnGetNewComponent( CameraComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct CubeCameraSystem : public IComponentSystem<CubeCameraComponent>
	{
	protected:
		virtual void OnGetNewComponent( CubeCameraComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct ShadowCasterSystem : public IComponentSystem<ShadowCasterComponent>
	{
	protected:
		virtual void OnGetNewComponent( ShadowCasterComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};

	struct LightSystem : public IComponentSystem<LightComponent>
	{
	protected:
		virtual void OnGetNewComponent( LightComponent* target ) override;
	public:
		virtual void OnUpdate( float elapsedTime ) override;
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};
}