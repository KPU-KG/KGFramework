#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "IRenderComponent.h"

namespace KG::Renderer
{
	struct RenderTexture;
	struct RenderTextureDesc;
};

namespace KG::Component
{
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

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc );

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

	};
};
