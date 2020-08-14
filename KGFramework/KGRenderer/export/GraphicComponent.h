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
//���� �ؽ�ó ���� ���� ��ũ���� ���� ��ġ
//���׸��� ��� ���̴� �ö󰥶� ����  CBV 2��
//������Ʈ ��� 1���� SRV�� ���� SRV 0��
//ī�޶�� ��� CBV 0��
//���̴� �н� ������ CBV 1��

//�ؽ�ó���� ū ��ũ���� ���� �ΰ��� �з��� �ɰ��� ���� (ť����� �ʿ��ϸ�?? ) 
//���� �����̽� 1�� SRV0 /  �����̽�2 �� SRV0 ���� �����ϴ� �迭
// ���׸��󿡼��� �ε��� ������ �� �����̽����� �ؽ�ó ���ܿ�
// 