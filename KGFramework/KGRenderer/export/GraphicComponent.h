#pragma once
#include <d3d12.h>
#include <vector>
#include <array>
#include <DirectXMath.h>
#include "IComponent.h"
#include "IRenderComponent.h"
#include "CameraComponent.h"

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
namespace KG::Utill
{
	class AnimationSet;
}

namespace KG::Component
{
	class TransformComponent;
	class MaterialComponent;
	class GeometryComponent;
	class Render3DComponent;
	class CubeCameraComponent;
	class CameraComponent;
	class BoneTransformComponent;

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

	class DLL GeometryComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend BoneTransformComponent;
		std::vector<KG::Renderer::Geometry*> geometrys;
	public:
		void InitializeGeometry( const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0 );
		bool HasBone() const;
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
		std::vector<KG::Renderer::Shader*> shaders;
		std::vector<KG::Renderer::MaterialConstant*> shaderDatas;
		virtual void OnDestroy() override;
		std::vector<UINT> materialIndexs;
	public:
		void InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex = 0 );
		void InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex = 0 );
		unsigned GetMaterialIndex(UINT slotIndex = 0) const;
	};

	/// @brief 메쉬의 본과 실제 게임오브젝트를 링크하고 애니메이션 트랜스폼 행렬을 갱신합니다.
	class DLL BoneTransformComponent : public IRenderComponent
	{
		friend Render3DComponent;
		using FrameCacheVector = std::vector<KG::Core::GameObject*>;
	protected:
		KG::Component::GeometryComponent* geometry = nullptr;
		std::vector<FrameCacheVector> frameCache;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
	public:
		KG::Core::GameObject* BoneIndexToGameObject( UINT index, UINT submeshIndex = 0 ) const;
		void InitializeBone( KG::Core::GameObject* rootNode );
	};

	class DLL AnimationStreamerComponent : public IRenderComponent
	{
		using FrameCacheVector = std::vector<KG::Core::GameObject*>;
	protected:
		KG::Utill::AnimationSet* anim = nullptr;
		KG::Component::GeometryComponent* geometry = nullptr;
		std::vector<FrameCacheVector> frameCache;
		float timer = 0.0f;
		float duration = 0.0f;
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		void MatchNode();
	public:
		virtual void Update( float timeElapsed ) override;
		void InitializeAnimation( const KG::Utill::HashString& animationId, UINT animationIndex = 0);
		void GetDuration();
	};

	REGISTER_COMPONENT_ID( LightComponent );
	REGISTER_COMPONENT_ID( CameraComponent );
	REGISTER_COMPONENT_ID( CubeCameraComponent );
	REGISTER_COMPONENT_ID( Render3DComponent );
	REGISTER_COMPONENT_ID( GeometryComponent );
	REGISTER_COMPONENT_ID( MaterialComponent );
	REGISTER_COMPONENT_ID( BoneTransformComponent );
	REGISTER_COMPONENT_ID( AnimationStreamerComponent );

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