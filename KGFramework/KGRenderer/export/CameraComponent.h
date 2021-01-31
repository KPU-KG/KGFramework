#pragma once
#include <DirectXMath.h>
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
		float farZ = 1000.0f;

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

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;

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
		std::array<CameraComponent, 6> cameras;

		KG::Renderer::RenderTexture* renderTexture = nullptr;

		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		bool isVisible = true;
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
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

	REGISTER_COMPONENT_ID( CameraComponent );
	REGISTER_COMPONENT_ID( CubeCameraComponent );

};
