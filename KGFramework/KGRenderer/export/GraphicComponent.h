#pragma once
#include "IComponent.h"
#include <vector>

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
	struct MaterialConstant;
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

	class DLL IRenderComponent : public IComponent
	{
	public:
		virtual void OnPreRender() {};
		virtual void OnRender() {};
	};

	class DLL CameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
	};


	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
		KG::Renderer::KGRenderJob* renderJob = nullptr;
	public:
		bool isVisible = true;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnRender() override;
		virtual void OnPreRender() override;
		void SetVisible( bool visible );
		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		void RegisterTransform( TransformComponent* transform );
		void RegisterMaterial( MaterialComponent* material );
		void RegisterGeometry( GeometryComponent* geometry );
	};

	class DLL GeometryComponent : public IRenderComponent
	{
		friend Render3DComponent;
		KG::Renderer::Geometry* geometry = nullptr;
	public:
		void InitializeGeometry( const KG::Utill::HashString& shaderID );
	};
	class DLL LightComponent : public IRenderComponent
	{
		friend Render3DComponent;

	};

	class DLL MaterialComponent : public IRenderComponent
	{
		friend Render3DComponent;
	protected:
		KG::System::ISystem* materialSystem = nullptr;
		KG::Renderer::Shader* shaders = nullptr;
		KG::Renderer::MaterialConstant* shaderDatas = nullptr;
		virtual void OnDestroy() override;
	public:
		void InitializeShader( const KG::Utill::HashString& shaderID );
		unsigned GetMaterialIndex() const;
	};

	REGISTER_COMPONENT_ID( KG::Component::IRenderComponent );
	REGISTER_COMPONENT_ID( KG::Component::CameraComponent );
	REGISTER_COMPONENT_ID( KG::Component::Render3DComponent );
	REGISTER_COMPONENT_ID( KG::Component::LightComponent );
	REGISTER_COMPONENT_ID( KG::Component::MaterialComponent );
	REGISTER_COMPONENT_ID( KG::Component::GeometryComponent );
}
//대충 텍스처 류는 전부 디스크립터 힙에 배치
//메테리얼 뷰는 셰이더 올라갈때 묶음  CBV 2번
//오브젝트 뷰는 1번에 SRV로 묶음 SRV 0번
//카메라는 상수 CBV 0번
//셰이더 패스 정보는 CBV 1번

//텍스처들은 큰 디스크립터 힙에 두가지 분류로 쪼개서 넣음 (큐브맵이 필요하면?? ) 
//각각 스페이스 1의 SRV0 /  스페이스2 의 SRV0 부터 시작하는 배열
// 메테리얼에서는 인덱스 참조로 각 스페이스에서 텍스처 떙겨옴
// 