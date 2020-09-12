#pragma once
#include <d3d12.h>
#include <vector>
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
	class RenderTexture;
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
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) {};
	};

	class DLL CameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		// User Data
		float fovY = 90.0f;
		float aspectRatio = 16.0f / 9.0f;
		float nearZ = 0.01f;
		float farZ = 1000.0f;

		ID3D12Resource* renderTarget = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle;
		D3D12_RESOURCE_STATES defaultRTState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

		TransformComponent* transform;
		ID3D12Resource* cameraDataBuffer = nullptr;

		struct CameraData;
		CameraData* cameraData = nullptr;
		CameraData* mappedCameraData = nullptr;

		bool ProjDirty = true;
		void OnProjDirty() { ProjDirty = true; }
		void RefreshCameraData();
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		bool isVisible = true;
		bool isMainCamera = true;
		//Viewport 설정 필요

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();

		void SetFovY( float value ) { OnProjDirty(); this->fovY = value; };
		void SetAspectRatio( float value ) { OnProjDirty(); this->aspectRatio = value; };
		void SetNearZ( float value ) { OnProjDirty(); this->nearZ = value; };
		void SetFarZ( float value ) { OnProjDirty(); this->farZ = value; };

		auto GetFovY() { return this->fovY; };
		auto GetAspectRatio() { return this->aspectRatio; };
		auto GetNearZ() { return this->nearZ; };
		auto GetFarZ() { return this->farZ; };

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void SetRenderTarget( ID3D12Resource* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle, D3D12_RESOURCE_STATES defaultRTState );

		auto GetRenderTarget() const
		{
			return this->renderTarget;
		}
		auto GetRenderTargetViewHandle() const
		{
			return this->renderTargetHandle;
		}
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
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
	public:
		bool isVisible = true;
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

	struct LightData
	{
		DirectX::XMFLOAT3 Strength;
		float FalloffStart;
		DirectX::XMFLOAT3 Direction;
		float FalloffEnd;
		DirectX::XMFLOAT3 Position;
		float SpotPower;
	};

	//struct LightRef
	//{
	//	DirectX::XMFLOAT3& Strength;
	//	float& FalloffStart;
	//	DirectX::XMFLOAT3& Direction;
	//	float& FalloffEnd;
	//	DirectX::XMFLOAT3& Position;
	//	float& SpotPower;
	//	LightRef( LightData& light )
	//		:
	//		Strength( light.Strength ),
	//		FalloffStart( light.FalloffStart ),
	//		Direction( light.Direction ),
	//		FalloffEnd( light.FalloffEnd ),
	//		Position( light.Position ),
	//		SpotPower( light.SpotPower )
	//	{
	//	};
	//};

	struct PointLightRef
	{
		DirectX::XMFLOAT3& Strength;
		float& FalloffStart;
		float& FalloffEnd;
		PointLightRef( LightData& light )
			: Strength( light.Strength ), FalloffStart( light.FalloffStart ), FalloffEnd( light.FalloffEnd )
		{
		};
	};
	struct DirectionalLightRef
	{
		DirectX::XMFLOAT3& Strength;
		DirectX::XMFLOAT3& Direction;
		DirectionalLightRef( LightData& light )
			: Strength( light.Strength ), Direction( light.Direction )
		{
		};
	};
	class DLL LightComponent : public IRenderComponent
	{
		KG::Renderer::KGRenderJob* renderJob = nullptr;
		TransformComponent* transform = nullptr;
		void SetRenderJob( KG::Renderer::KGRenderJob* renderJob );
		bool isDirty = true;
		LightData light;
		KG::Renderer::Shader* currentShader = nullptr;
		KG::Renderer::Geometry* currentGeometry = nullptr;
		void RegisterTransform( TransformComponent* transform );

		inline static KG::Renderer::Shader* directionalLightShader = nullptr;
		inline static KG::Renderer::Shader* spotLightShader = nullptr;
		inline static KG::Renderer::Shader* pointLightShader = nullptr;

		inline static KG::Renderer::Geometry* directionalLightGeometry = nullptr;
		inline static KG::Renderer::Geometry* spotLightGeometry = nullptr;
		inline static KG::Renderer::Geometry* pointLightGeometry = nullptr;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
	public:
		void SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction );
		void SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd );
		DirectionalLightRef GetDirectionalLightRef();
		PointLightRef GetPointLightRef();

		void UpdateChanged();

		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commadList ) override;
		virtual void OnPreRender() override;
		void SetVisible( bool visible );
	};

	class DLL MaterialComponent : public IRenderComponent
	{
		friend Render3DComponent;
	protected:
		KG::System::ISystem* materialSystem = nullptr;
		KG::Renderer::Shader* shaders = nullptr;
		KG::Renderer::MaterialConstant* shaderDatas = nullptr;
		virtual void OnDestroy() override;
		size_t materialIndex = 0;
	public:
		void InitializeMaterial( const KG::Utill::HashString& materialID );
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