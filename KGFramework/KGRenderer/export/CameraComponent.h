#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	struct RenderTexture;
	struct RenderTextureDesc;
};

namespace KG::Component
{
	class TransformComponent;
	class Render3DComponent;
	class ShadowCasterComponent;
	class CubeCameraComponent;
	class LightComponent;
	class ShadowCasterComponent;

	struct CameraData
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
		DirectX::XMFLOAT4X4 viewProjection;

		DirectX::XMFLOAT4X4 inverseView;
		DirectX::XMFLOAT4X4 inverseProjection;
		DirectX::XMFLOAT4X4 inverseViewProjection;

		DirectX::XMFLOAT3 cameraWorldPosition;
		float pad0;
		DirectX::XMFLOAT3 look;
		float pad1;
	};

	class DLL CameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;
		friend CubeCameraComponent;

		// User Data
		float fovY = 90.0f;
		float aspectRatio = 16.0f / 9.0f;
		float nearZ = 0.01f;
		float farZ = 500.0f;

		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;

		bool isRenderTexureCreatedInCamera = false;
		KG::Renderer::RenderTexture* renderTexture = nullptr;

		TransformComponent* transform;
		ID3D12Resource* cameraDataBuffer = nullptr;

		bool isCubeRenderer = false;
		int cubeIndex = 0;

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
		void SetViewport( const D3D12_VIEWPORT& viewport ) { this->viewport = viewport; };
		void SetScissorRect( const D3D12_RECT& rect ) { this->scissorRect = rect; };
		void SetDefaultRender();
		void SetCubeRender( int index );

		auto GetFovY() const { return this->fovY; };
		auto GetAspectRatio() const { return this->aspectRatio; };
		auto GetNearZ() const { return this->nearZ; };
		auto GetFarZ() const { return this->farZ; };
		auto GetViewport() const { return this->viewport; };
		auto GetScissorRect() const { return this->scissorRect; };
		auto GetCubeIndex() const { return this->cubeIndex; };
		DirectX::XMFLOAT4X4 GetView();
		DirectX::XMFLOAT4X4 GetProj();
		DirectX::XMFLOAT4X4 GetViewProj();

		DirectX::BoundingFrustum GetFrustum();
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void SetRenderTexture( KG::Renderer::RenderTexture* renderTexture, int index = 0 );
		void InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc );

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

	};

	class DLL CubeCameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;
		std::array<CameraComponent, 6> cameras;

		KG::Renderer::RenderTexture* renderTexture = nullptr;

		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;
		void InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc );

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

		auto& GetCameras()
		{
			return this->cameras;
		}
	};

	struct GSCubeCameraData
	{
		DirectX::XMFLOAT4X4 view[6];
		DirectX::XMFLOAT4X4 projection;

		DirectX::XMFLOAT4X4 inverseView[6];
		DirectX::XMFLOAT4X4 inverseProjection;

		DirectX::XMFLOAT3 cameraWorldPosition;
		float pad0;

		DirectX::XMFLOAT4 look[6];
	};

	class DLL GSCubeCameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;

		float nearZ = 0.01f;
		float farZ = 1000.0f;

		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;

		KG::Renderer::RenderTexture* renderTexture = nullptr;

		TransformComponent* transform;

		ID3D12Resource* cameraDataBuffer = nullptr;

		GSCubeCameraData* cameraData = nullptr;
		GSCubeCameraData* mappedCameraData = nullptr;

		bool ProjDirty = true;

		void OnProjDirty() { ProjDirty = true; }
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;

	public:
		static constexpr float fovY = 90.0f;
		static constexpr float aspectRatio = 1.0f / 1.0f;

		bool isVisible = true;
		bool isMainCamera = true;
		//Viewport 설정 필요

		void RefreshCameraData();

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();

		void SetNearZ( float value ) { OnProjDirty(); this->nearZ = value; };
		void SetFarZ( float value ) { OnProjDirty(); this->farZ = value; };
		void SetDefaultRender();

		auto GetNearZ() const { return this->nearZ; };
		auto GetFarZ() const { return this->farZ; };

		void SetViewport( const D3D12_VIEWPORT& viewport ) { this->viewport = viewport; };
		void SetScissorRect( const D3D12_RECT& rect ) { this->scissorRect = rect; };

		auto GetViewport() const { return this->viewport; };
		auto GetScissorRect() const { return this->scissorRect; };

		DirectX::XMFLOAT4X4 GetView(size_t index);
		DirectX::XMFLOAT4X4 GetProj();
		DirectX::XMFLOAT4X4 GetViewProj( size_t index );

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;
		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc );

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

	};


	constexpr size_t cascadeLevel = 4;
	struct GSCascadeCameraData
	{
		DirectX::XMFLOAT4X4 view[cascadeLevel];
		DirectX::XMFLOAT4X4 projection[cascadeLevel];

		DirectX::XMFLOAT4 cameraWorldPosition[cascadeLevel];
		DirectX::XMFLOAT4 look;
	};

	//0번 -> 전역 라이팅 / 1번 -> CSM 0번 / 2번 -> CSM 1번 / 3번 -> CSM 2번
	class DLL GSCascadeCameraComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;

		float nearZ = 0.01f;
		float farZ = 1000.0f;

		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;


		KG::Renderer::RenderTexture* renderTexture = nullptr;

		TransformComponent* transform;
		CameraComponent* mainCamera;
		LightComponent* directionalLight;

		ID3D12Resource* cameraDataBuffer = nullptr;

		GSCascadeCameraData* cameraData = nullptr;
		GSCascadeCameraData* mappedCameraData = nullptr;

		bool ProjDirty = true;

		void OnProjDirty() { ProjDirty = true; }
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;

	public:
		static constexpr float fovY = 90.0f;
		static constexpr float aspectRatio = 1.0f / 1.0f;

		bool isVisible = true;
		bool isMainCamera = true;
		//Viewport 설정 필요

		void RefreshCameraData();

		void SetNearZ( float value ) { OnProjDirty(); this->nearZ = value; };
		void SetFarZ( float value ) { OnProjDirty(); this->farZ = value; };
		void SetDefaultRender();

		auto GetNearZ() const { return this->nearZ; };
		auto GetFarZ() const { return this->farZ; };

		void SetViewport( const D3D12_VIEWPORT& viewport ) { this->viewport = viewport; };
		void SetScissorRect( const D3D12_RECT& rect ) { this->scissorRect = rect; };

		auto GetViewport() const { return this->viewport; };
		auto GetScissorRect() const { return this->scissorRect; };

		DirectX::XMFLOAT4X4 GetView( size_t index );
		DirectX::XMFLOAT4X4 GetProj( size_t index );
		std::array<DirectX::XMFLOAT4X4, 4> GetViewProj();
		DirectX::XMFLOAT4X4 GetViewProj( size_t viewIndex, size_t projIndex );

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void InitalizeCascade( KG::Component::CameraComponent* camera, KG::Component::LightComponent* light );
		void RefreshNormalViewProj();
		void RefreshCascadeViewProj();

		void InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc );

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

	};


	REGISTER_COMPONENT_ID( CameraComponent );
	REGISTER_COMPONENT_ID( CubeCameraComponent );
	REGISTER_COMPONENT_ID( GSCubeCameraComponent );
	REGISTER_COMPONENT_ID( GSCascadeCameraComponent );

};
