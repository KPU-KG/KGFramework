#pragma once
#include <d3d12.h>
#include <vector>
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
		virtual void OnRender( ID3D12GraphicsCommandList* commadList) {};
	};

	class DLL CameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		// User Data
		float fovY = 90.0f;
		float aspectRatio = 16.0f/9.0f;
		float nearZ = 0.01f;
		float farZ = 1000.0f;

		TransformComponent* transform;
		ID3D12Resource* cameraBuffer = nullptr;

		struct CameraData;
		CameraData* cameraData = nullptr;
		CameraData* mappedCameraData = nullptr;

		bool ProjDirty = true;
		void OnProjDirty() { ProjDirty = true; }
		void RefreshCameraData();
	public:
		bool isVisible = true;

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();

		void SetFovY( float value ) { OnProjDirty(); this->fovY = value; };
		void SetAspectRatio( float value ) { OnProjDirty(); this->aspectRatio = value; };
		void SetNearZ( float value ) { OnProjDirty(); this->nearZ = value; };
		void SetFarZ( float value ) { OnProjDirty(); this->farZ = value; };

		auto GetFovY( ) { return this->fovY; };
		auto GetAspectRatio( ) { return this->aspectRatio; };
		auto GetNearZ( ) { return this->nearZ; };
		auto GetFarZ( ) { return this->farZ; };

		virtual void OnDestroy() override;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
	};


	class DLL Render3DComponent : public IRenderComponent
	{
		TransformComponent* transform = nullptr;
		GeometryComponent* geometry = nullptr;
		MaterialComponent* material = nullptr;
		KG::Renderer::KGRenderJob* renderJob = nullptr;

		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		void RegisterTransform( TransformComponent* transform );
		void RegisterMaterial( MaterialComponent* material );
		void RegisterGeometry( GeometryComponent* geometry );
	public:
		bool isVisible = true;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
		void SetVisible( bool visible );
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
	REGISTER_COMPONENT_ID( LightComponent );
	REGISTER_COMPONENT_ID( CameraComponent );
	REGISTER_COMPONENT_ID( Render3DComponent );
	REGISTER_COMPONENT_ID( GeometryComponent );
	REGISTER_COMPONENT_ID( MaterialComponent );

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