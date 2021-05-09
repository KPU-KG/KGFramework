#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <array>
#include "IRenderComponent.h"
#include "RendererDesc.h"
#include "SerializableProperty.h"
#include "ICameraComponent.h"

namespace KG::Component
{
	class FakeCameraComponent : public ICameraComponent
	{
    public:
        // ICameraComponent을(를) 통해 상속됨
        virtual void SetFovY(float value) override;
        virtual void SetAspectRatio(float value) override;
        virtual void SetNearZ(float value) override;
        virtual void SetFarZ(float value) override;
        virtual void SetViewport(const CameraViewport& viewport) override;
        virtual void SetScissorRect(const RECT& rect) override;
        virtual void SetDefaultRender() override;
        virtual float GetFovY() const override;
        virtual float GetAspectRatio() const override;
        virtual float GetNearZ() const override;
        virtual float GetFarZ() const override;
        virtual CameraViewport GetViewport() const override;
        virtual RECT GetScissorRect() const override;
        virtual DirectX::XMFLOAT4X4 GetView() override;
        virtual DirectX::XMFLOAT4X4 GetProj() override;
        virtual DirectX::XMFLOAT4X4 GetViewProj() override;
        virtual DirectX::BoundingFrustum GetFrustum() override;
        virtual void OnPreRender() override;
        virtual void SetMainCamera() override;
        virtual bool IsMainCamera() const override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };

	class FakeCubeCameraComponent : public ICubeCameraComponent
	{
    public:
        // ICubeCameraComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnDestroy() override;
        virtual void OnPreRender() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
    };

	class FakeGSCubeCameraComponent : public IGSCubeCameraComponent
	{
    public:
        // IGSCubeCameraComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnDestroy() override;
        virtual void SetNearZ(float value) override;
        virtual void SetFarZ(float value) override;
        virtual void SetDefaultRender() override;
        virtual float GetNearZ() const override;
        virtual float GetFarZ() const override;
        virtual void SetViewport(const CameraViewport& viewport) override;
        virtual void SetScissorRect(const RECT& rect) override;
        virtual CameraViewport GetViewport() const override;
        virtual RECT GetScissorRect() const override;
        virtual DirectX::XMFLOAT4X4 GetView(size_t index) override;
        virtual DirectX::XMFLOAT4X4 GetProj() override;
        virtual DirectX::XMFLOAT4X4 GetViewProj(size_t index) override;
        virtual void OnPreRender() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
	class FakeGSCascadeCameraComponent : public IGSCascadeCameraComponent
	{
    public:
        // IGSCascadeCameraComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnDestroy() override;
        virtual void SetNearZ(float value) override;
        virtual void SetFarZ(float value) override;
        virtual void SetDefaultRender() override;
        virtual float GetNearZ() const override;
        virtual float GetFarZ() const override;
        virtual void SetViewport(const CameraViewport& viewport) override;
        virtual void SetScissorRect(const RECT& rect) override;
        virtual CameraViewport GetViewport() const override;
        virtual RECT GetScissorRect() const override;
        virtual DirectX::XMFLOAT4X4 GetView(size_t index) override;
        virtual DirectX::XMFLOAT4X4 GetProj(size_t index) override;
        virtual std::array<DirectX::XMFLOAT4X4, 4> GetViewProj() override;
        virtual DirectX::XMFLOAT4X4 GetViewProj(size_t viewIndex, size_t projIndex) override;
        virtual void OnPreRender() override;
        virtual void InitalizeCascade(KG::Component::ICameraComponent* directionalLightCamera, KG::Component::ILightComponent* light) override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
    };


	REGISTER_COMPONENT_ID_REPLACE( FakeCameraComponent, ICameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( FakeCubeCameraComponent, ICubeCameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( FakeGSCubeCameraComponent, IGSCubeCameraComponent);
	REGISTER_COMPONENT_ID_REPLACE( FakeGSCascadeCameraComponent, IGSCascadeCameraComponent);

};
