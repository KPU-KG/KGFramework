#pragma once
#include <vector>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	struct KGRenderJob;
};

namespace KG::Component
{
	class TransformComponent;
	class GeometryComponent;
	class MaterialComponent;
	class BoneTransformComponent;
	class CubeCameraComponent;

	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
		BoneTransformComponent* boneAnimation = nullptr;
		CubeCameraComponent* reflectionProbe = nullptr;
		std::vector<KG::Renderer::KGRenderJob*> renderJobs;
		std::vector<UINT> jobMaterialIndexs;
		void AddRenderJob( KG::Renderer::KGRenderJob* renderJob, UINT materialIndex );
		void RegisterTransform( TransformComponent* transform );
		void RegisterMaterial( MaterialComponent* material );
		void RegisterGeometry( GeometryComponent* geometry );
		void RegisterBoneAnimation( BoneTransformComponent* anim );
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
	public:
		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
		void SetVisible( bool visible );
		void SetReflectionProbe( CubeCameraComponent* probe );
	};
	REGISTER_COMPONENT_ID( Render3DComponent );
};
