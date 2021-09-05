#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "IRender3DComponent.h"
#include "IDXRenderComponent.h"

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

	class Render3DComponent : public IRender3DComponent, IDXRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
		BoneTransformComponent* boneAnimation = nullptr;
		CubeCameraComponent* reflectionProbe = nullptr;
		std::vector<KG::Renderer::KGRenderJob*> renderJobs;
        std::vector<bool> jobCulled;
		std::vector<UINT> jobMaterialIndexs;
		void AddRenderJob( KG::Renderer::KGRenderJob* renderJob, UINT materialIndex );
		void RegisterTransform( TransformComponent* transform );
		void RegisterMaterial( MaterialComponent* material );
		void RegisterGeometry( GeometryComponent* geometry );
		void RegisterBoneAnimation( BoneTransformComponent* anim );
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
        int cullingIndex = -1;
        int updateIndex = -1;
        std::vector<UINT> dxrIndexs;
	public:
		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
        virtual void CullingProcess(const DirectX::BoundingFrustum& frustum);
		virtual void SetVisible( bool visible ) override;
        void OnPostDXR();
        void OnUpdateTLAS(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
		void SetReflectionProbe( CubeCameraComponent* probe );
		void RemoveJobs();
		virtual void ReloadRender() override;
		//Serialize Part
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();

        // IRender3DComponent을(를) 통해 상속됨
        virtual bool GetVisible() const override;
        virtual void SetReflectionProbe(ICubeCameraComponent* probe) override;
    };
	//REGISTER_COMPONENT_ID( Render3DComponent );
    REGISTER_COMPONENT_ID_REPLACE(Render3DComponent, IRender3DComponent);
};
