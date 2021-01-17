#pragma once
#include <d3d12.h>
#include <vector>
#include <array>
#include <DirectXMath.h>
#include "IComponent.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // GRAPHICPART_EXPORTS

namespace KG::Core
{
	class GameObject;
};
namespace KG::Renderer
{
	class KGRenderJob;
	class Shader;
	class Geometry;
	struct RenderTexture;
	struct RenderTextureDesc;
	struct MaterialConstant;
	struct RendererSetting;
}
namespace KG::System
{
	class ISystem;
}

namespace KG::Component
{
	class TransformComponent;
	class MaterialComponent;
	class GeometryComponent;
	class Render3DComponent;
	class CubeCameraComponent;
	class CameraComponent;
	class AnimationComponent;
	class IRenderComponent;

	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) {};
	};

	// class DLL Render3DComponent : public IRenderComponent
	// {
	// 	TransformComponent* transform = nullptr;
	// 	GeometryComponent* geometry = nullptr;
	// 	MaterialComponent* material = nullptr;
	// 	CubeCameraComponent* reflectionProbe = nullptr;
	// 	std::vector<KG::Renderer::KGRenderJob*> renderJobs;
	// 	std::vector<UINT> jobMaterialIndexs;
	// 	void AddRenderJob(KG::Renderer::KGRenderJob* renderJob, UINT materialIndex);
	// 	void RegisterTransform(TransformComponent* transform);
	// 	void RegisterMaterial(MaterialComponent* material);
	// 	void RegisterGeometry(GeometryComponent* geometry);
	// 	virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	// public:
	// 	bool isVisible = true;
	// 	virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
	// 	virtual void OnPreRender() override;
	// 	void SetVisible(bool visible);
	// 	void SetReflectionProbe(CubeCameraComponent* probe);
	// };
	// 
	// class DLL GeometryComponent : public IRenderComponent
	// {
	// 	friend Render3DComponent;
	// 	std::vector<KG::Renderer::Geometry*> geometrys;
	// public:
	// 	void InitializeGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0);
	// };

	class DLL AnimationComponent : public IRenderComponent {
	protected:
	public:

	};

	// REGISTER_COMPONENT_ID(LightComponent);
	// REGISTER_COMPONENT_ID(CameraComponent);
	// REGISTER_COMPONENT_ID(CubeCameraComponent);
	// REGISTER_COMPONENT_ID(Render3DComponent);
	// REGISTER_COMPONENT_ID(GeometryComponent);
	// REGISTER_COMPONENT_ID(MaterialComponent);
	//REGISTER_COMPONENT_ID( AvatarComponent );
}