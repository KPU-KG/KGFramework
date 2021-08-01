#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include "IRenderComponent.h"
#include "RendererDesc.h"
#include "SerializableProperty.h"
#include "ICameraComponent.h"
#include "IDXRenderComponent.h"

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
		double gameTime;
	};

    static D3D12_VIEWPORT CameraViewportToDX(const CameraViewport& b)
    {
        D3D12_VIEWPORT a;
        a.Height = b.Height;
        a.MaxDepth = b.MaxDepth;
        a.MinDepth = b.MinDepth;
        a.TopLeftX = b.TopLeftX;
        a.TopLeftY = b.TopLeftY;
        a.Width = b.Width;
        return a;
    }


    static CameraViewport DXToCameraViewport(const D3D12_VIEWPORT& b)
    {
        CameraViewport a;
        a.Height = b.Height;
        a.MaxDepth = b.MaxDepth;
        a.MinDepth = b.MinDepth;
        a.TopLeftX = b.TopLeftX;
        a.TopLeftY = b.TopLeftY;
        a.Width = b.Width;
        return a;
    }


	class CameraComponent : public ICameraComponent, IDXRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;
		friend CubeCameraComponent;

		// User Data
		float fovY = 90.0f;
		float aspectRatio = 16.0f / 9.0f;
		float nearZ = 0.01f;
		float farZ = 100.0f;

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

		CameraComponent();

		bool isCreateRenderTexture = true;

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
        ID3D12Resource* GetCameraDataBuffer() const { return this->cameraDataBuffer; };

		virtual void SetFovY( float value ) override { OnProjDirty(); this->fovY = value; };
		virtual void SetAspectRatio( float value ) override { OnProjDirty(); this->aspectRatio = value; };
		virtual void SetNearZ( float value ) override { OnProjDirty(); this->nearZ = value; };
		virtual void SetFarZ( float value ) override { OnProjDirty(); this->farZ = value; };
		virtual void SetViewport( const CameraViewport& viewport ) override { this->viewport = CameraViewportToDX(viewport); };
		virtual void SetScissorRect( const RECT& rect ) override { this->scissorRect = rect; };
		virtual void SetDefaultRender();
		void SetCubeRender( int index );

		virtual float GetFovY() const override { return this->fovY; };
		virtual float GetFarZ() const override { return this->farZ; };
		virtual float GetNearZ() const override { return this->nearZ; };
		virtual float GetAspectRatio() const override { return this->aspectRatio; };
		virtual CameraViewport GetViewport() const override { return DXToCameraViewport( this->viewport ); };
		virtual RECT GetScissorRect() const override { return this->scissorRect; };

		virtual DirectX::XMFLOAT4X4 GetView() override;
		virtual DirectX::XMFLOAT4X4 GetProj() override;
		virtual DirectX::XMFLOAT4X4 GetViewProj() override;

		virtual DirectX::BoundingFrustum GetFrustum() override;
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;

		void SetMainCamera();

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void SetRenderTexture( KG::Renderer::RenderTexture* renderTexture, int index = 0 );
		void InitializeRenderTexture();

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}
        auto GetCubeIndex() const
        {
            return this->cubeIndex;
        }

		//Serialize Part
	private:
		KG::Renderer::RenderTextureProperty renderTextureProperty;
		KG::Core::SerializableProperty<bool> mainCameraProp;
		KG::Core::SerializableProperty<float> fovYProp;
		KG::Core::SerializableProperty<float> aspectRatioProp;
		KG::Core::SerializableProperty<float> nearZProp;
		KG::Core::SerializableProperty<float> farZProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();

        // ICameraComponent을(를) 통해 상속됨
        virtual bool IsMainCamera() const override;
    };

	class CubeCameraComponent : public ICubeCameraComponent, IDXRenderComponent
	{
		friend Render3DComponent;
		friend ShadowCasterComponent;
		std::array<CameraComponent, 6> cameras;

		KG::Renderer::RenderTexture* renderTexture = nullptr;

		virtual void OnCreate( KG::Core::GameObject* gameObject ) override;
		virtual void OnDestroy() override;
	public:
		bool isVisible = true;
		CubeCameraComponent();
		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;
		void InitializeRenderTexture();

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}

		auto& GetCameras()
		{
			return this->cameras;
		}

	private:
		KG::Renderer::RenderTextureProperty renderTextureProperty;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
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

	class GSCubeCameraComponent : public IGSCubeCameraComponent, IDXRenderComponent
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

		GSCubeCameraComponent();

		bool isVisible = true;
		bool isMainCamera = true;
		//Viewport 설정 필요

		void RefreshCameraData();

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();

		virtual void SetNearZ( float value ) override { OnProjDirty(); this->nearZ = value; } ;
		virtual void SetFarZ( float value ) override { OnProjDirty(); this->farZ = value; };
		virtual void SetDefaultRender() override;

        virtual float GetFarZ() const override { return this->farZ; };
        virtual float GetNearZ() const override { return this->nearZ; };

        virtual void SetViewport(const CameraViewport& viewport) override { this->viewport = CameraViewportToDX(viewport); };
        virtual void SetScissorRect(const RECT& rect) override { this->scissorRect = rect; };

        virtual CameraViewport GetViewport() const override { return DXToCameraViewport(this->viewport); };
        virtual RECT GetScissorRect() const override { return this->scissorRect; };

		virtual DirectX::XMFLOAT4X4 GetView(size_t index) override;
		virtual DirectX::XMFLOAT4X4 GetProj() override;
		virtual DirectX::XMFLOAT4X4 GetViewProj( size_t index ) override;

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;
		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

		void InitializeRenderTexture();

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}
		//Serialize Part
	private:
		KG::Renderer::RenderTextureProperty renderTextureProperty;
		KG::Core::SerializableProperty<float> nearZProp;
		KG::Core::SerializableProperty<float> farZProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
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
	class GSCascadeCameraComponent : public IGSCascadeCameraComponent, IDXRenderComponent
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

		KG::Renderer::RenderTextureDesc renderTextureDesc;

        float cascadePoint[5] = { 0.0f, 0.01f, 0.025f, 0.1f, 1.0f };

		//Viewport 설정 필요

		GSCascadeCameraComponent();

		void RefreshCameraData();

        virtual void SetNearZ(float value) override { OnProjDirty(); this->nearZ = value; };
        virtual void SetFarZ(float value) override { OnProjDirty(); this->farZ = value; };
        virtual void SetDefaultRender() override;

        virtual float GetFarZ() const override { return this->farZ; };
        virtual float GetNearZ() const override { return this->nearZ; };

        virtual void SetViewport(const CameraViewport& viewport) override { this->viewport = CameraViewportToDX(viewport); };
        virtual void SetScissorRect(const RECT& rect) override { this->scissorRect = rect; };

        virtual CameraViewport GetViewport() const override { return DXToCameraViewport(this->viewport); };
        virtual RECT GetScissorRect() const override { return this->scissorRect; };

		virtual DirectX::XMFLOAT4X4 GetView( size_t index ) override;
		virtual DirectX::XMFLOAT4X4 GetProj( size_t index ) override;
		virtual std::array<DirectX::XMFLOAT4X4, 4> GetViewProj() override;
		virtual DirectX::XMFLOAT4X4 GetViewProj( size_t viewIndex, size_t projIndex ) override;

		virtual void OnRender( ID3D12GraphicsCommandList* commandList ) override;
		virtual void OnPreRender() override;

		void SetCameraRender( ID3D12GraphicsCommandList* commandList );
		void EndCameraRender( ID3D12GraphicsCommandList* commandList );

        virtual void InitalizeCascade(KG::Component::ICameraComponent* directionalLightCamera, KG::Component::ILightComponent* light) override;
        void InitalizeCascade(KG::Component::CameraComponent* directionalLightCamera, KG::Component::LightComponent* light);
        void RefreshNormalViewProj();
		void RefreshCascadeViewProj();

		void InitializeRenderTexture();

		auto& GetRenderTexture()
		{
			return *this->renderTexture;
		}
		//Serialize Part
	private:
		KG::Renderer::RenderTextureProperty renderTextureProperty;
		KG::Core::SerializableProperty<float> nearZProp;
		KG::Core::SerializableProperty<float> farZProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);

        // IGSCascadeCameraComponent을(를) 통해 상속됨
        virtual void SetMainCamera(ICameraComponent* camera) override;
    };


	REGISTER_COMPONENT_ID_REPLACE( CameraComponent, ICameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( CubeCameraComponent, ICubeCameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( GSCubeCameraComponent, IGSCubeCameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( GSCascadeCameraComponent, IGSCascadeCameraComponent);

};
