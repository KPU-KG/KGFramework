#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include "IRenderComponent.h"
#include "RendererDesc.h"
#include "SerializableProperty.h"

namespace KG::Component
{
    class ILightComponent;
    struct CameraViewport
    {
        float TopLeftX;
        float TopLeftY;
        float Width;
        float Height;
        float MinDepth;
        float MaxDepth;
    };

	class DLL ICameraComponent : public IRenderComponent
	{
	public:
        KG::Renderer::RenderTextureDesc renderTextureDesc;
        virtual void SetFovY(float value) = 0;
		virtual void SetAspectRatio( float value ) = 0;
		virtual void SetNearZ( float value ) = 0;
		virtual void SetFarZ( float value ) = 0;
        virtual void SetViewport(const CameraViewport& viewport) = 0;
        virtual void SetScissorRect(const RECT& rect) = 0;
		virtual void SetDefaultRender() = 0;

		virtual float GetFovY() const = 0;
		virtual float GetAspectRatio() const = 0;
		virtual float GetNearZ() const = 0;
		virtual float GetFarZ() const = 0;
        virtual CameraViewport GetViewport() const = 0;
        virtual RECT GetScissorRect() const = 0;

		virtual DirectX::XMFLOAT4X4 GetView() = 0;
		virtual DirectX::XMFLOAT4X4 GetProj() = 0;
		virtual DirectX::XMFLOAT4X4 GetViewProj() = 0;

		virtual DirectX::BoundingFrustum GetFrustum() = 0;
		virtual void OnPreRender() override = 0;

		virtual void SetMainCamera() = 0;
        virtual bool IsMainCamera() const = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};

	class DLL ICubeCameraComponent : public IRenderComponent
	{
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override = 0;
		virtual void OnDestroy() override = 0;
	public:
        KG::Renderer::RenderTextureDesc renderTextureDesc;
        virtual void OnPreRender() override = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
	};

	class DLL IGSCubeCameraComponent : public IRenderComponent
	{
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override = 0;
		virtual void OnDestroy() override = 0;

	public:
		static constexpr float fovY = 90.0f;
		static constexpr float aspectRatio = 1.0f / 1.0f;

		KG::Renderer::RenderTextureDesc renderTextureDesc;

		virtual void SetNearZ( float value ) = 0;
		virtual void SetFarZ( float value ) = 0;
		virtual void SetDefaultRender() = 0;

        virtual float GetNearZ() const = 0;
        virtual float GetFarZ() const = 0;

        virtual void SetViewport(const CameraViewport& viewport) = 0;
        virtual void SetScissorRect(const RECT& rect) = 0;

        virtual CameraViewport GetViewport() const = 0;
        virtual RECT GetScissorRect() const = 0;

		virtual DirectX::XMFLOAT4X4 GetView(size_t index) = 0;
		virtual DirectX::XMFLOAT4X4 GetProj() = 0;
		virtual DirectX::XMFLOAT4X4 GetViewProj( size_t index ) = 0;

		virtual void OnPreRender() override = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};


	class DLL IGSCascadeCameraComponent : public IRenderComponent
	{
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override = 0;
		virtual void OnDestroy() override = 0;

	public:
		KG::Renderer::RenderTextureDesc renderTextureDesc;

        virtual void SetNearZ(float value) = 0;
        virtual void SetFarZ(float value) = 0;
        virtual void SetDefaultRender() = 0;

        virtual float GetNearZ() const = 0;
        virtual float GetFarZ() const = 0;

        virtual void SetViewport(const CameraViewport& viewport) = 0;
        virtual void SetScissorRect(const RECT& rect) = 0;

        virtual CameraViewport GetViewport() const = 0;
        virtual RECT GetScissorRect() const = 0;

		virtual DirectX::XMFLOAT4X4 GetView( size_t index ) = 0;
		virtual DirectX::XMFLOAT4X4 GetProj( size_t index ) = 0;
		virtual std::array<DirectX::XMFLOAT4X4, 4> GetViewProj() = 0;
		virtual DirectX::XMFLOAT4X4 GetViewProj( size_t viewIndex, size_t projIndex ) = 0;

		virtual void OnPreRender() override = 0;

		virtual void InitalizeCascade( KG::Component::ICameraComponent* directionalLightCamera, KG::Component::ILightComponent* light ) = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
	};


	REGISTER_COMPONENT_ID( ICameraComponent );
	REGISTER_COMPONENT_ID( ICubeCameraComponent );
	REGISTER_COMPONENT_ID( IGSCubeCameraComponent );
	REGISTER_COMPONENT_ID( IGSCascadeCameraComponent );

};
