#include "FakeCameraComponent.h"

void KG::Component::FakeCameraComponent::SetFovY(float value)
{
}

void KG::Component::FakeCameraComponent::SetAspectRatio(float value)
{
}

void KG::Component::FakeCameraComponent::SetNearZ(float value)
{
}

void KG::Component::FakeCameraComponent::SetFarZ(float value)
{
}

void KG::Component::FakeCameraComponent::SetViewport(const CameraViewport& viewport)
{
}

void KG::Component::FakeCameraComponent::SetScissorRect(const RECT& rect)
{
}

void KG::Component::FakeCameraComponent::SetDefaultRender()
{
}

float KG::Component::FakeCameraComponent::GetFovY() const
{
    return 0.0f;
}

float KG::Component::FakeCameraComponent::GetAspectRatio() const
{
    return 0.0f;
}

float KG::Component::FakeCameraComponent::GetNearZ() const
{
    return 0.0f;
}

float KG::Component::FakeCameraComponent::GetFarZ() const
{
    return 0.0f;
}

KG::Component::CameraViewport KG::Component::FakeCameraComponent::GetViewport() const
{
    return CameraViewport();
}

RECT KG::Component::FakeCameraComponent::GetScissorRect() const
{
    return RECT();
}

DirectX::XMFLOAT4X4 KG::Component::FakeCameraComponent::GetView()
{
    return DirectX::XMFLOAT4X4();
}

DirectX::XMFLOAT4X4 KG::Component::FakeCameraComponent::GetProj()
{
    return DirectX::XMFLOAT4X4();
}

DirectX::XMFLOAT4X4 KG::Component::FakeCameraComponent::GetViewProj()
{
    return DirectX::XMFLOAT4X4();
}

DirectX::BoundingFrustum KG::Component::FakeCameraComponent::GetFrustum()
{
    return DirectX::BoundingFrustum();
}

void KG::Component::FakeCameraComponent::OnPreRender()
{
}

void KG::Component::FakeCameraComponent::SetMainCamera()
{
}

bool KG::Component::FakeCameraComponent::IsMainCamera() const
{
    return false;
}

void KG::Component::FakeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeCameraComponent::OnDrawGUI()
{
    return false;
}

void KG::Component::FakeCubeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeCubeCameraComponent::OnDestroy()
{
}

void KG::Component::FakeCubeCameraComponent::OnPreRender()
{
}

void KG::Component::FakeCubeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeCubeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

void KG::Component::FakeGSCubeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeGSCubeCameraComponent::OnDestroy()
{
}

void KG::Component::FakeGSCubeCameraComponent::SetNearZ(float value)
{
}

void KG::Component::FakeGSCubeCameraComponent::SetFarZ(float value)
{
}

void KG::Component::FakeGSCubeCameraComponent::SetDefaultRender()
{
}

float KG::Component::FakeGSCubeCameraComponent::GetNearZ() const
{
    return 0.0f;
}

float KG::Component::FakeGSCubeCameraComponent::GetFarZ() const
{
    return 0.0f;
}

void KG::Component::FakeGSCubeCameraComponent::SetViewport(const CameraViewport& viewport)
{
}

void KG::Component::FakeGSCubeCameraComponent::SetScissorRect(const RECT& rect)
{
}

KG::Component::CameraViewport KG::Component::FakeGSCubeCameraComponent::GetViewport() const
{
    return CameraViewport();
}

RECT KG::Component::FakeGSCubeCameraComponent::GetScissorRect() const
{
    return RECT();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCubeCameraComponent::GetView(size_t index)
{
    return DirectX::XMFLOAT4X4();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCubeCameraComponent::GetProj()
{
    return DirectX::XMFLOAT4X4();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCubeCameraComponent::GetViewProj(size_t index)
{
    return DirectX::XMFLOAT4X4();
}

void KG::Component::FakeGSCubeCameraComponent::OnPreRender()
{
}

void KG::Component::FakeGSCubeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeGSCubeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeGSCubeCameraComponent::OnDrawGUI()
{
    return false;
}

void KG::Component::FakeGSCascadeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeGSCascadeCameraComponent::OnDestroy()
{
}

void KG::Component::FakeGSCascadeCameraComponent::SetNearZ(float value)
{
}

void KG::Component::FakeGSCascadeCameraComponent::SetFarZ(float value)
{
}

void KG::Component::FakeGSCascadeCameraComponent::SetDefaultRender()
{
}

float KG::Component::FakeGSCascadeCameraComponent::GetNearZ() const
{
    return 0.0f;
}

float KG::Component::FakeGSCascadeCameraComponent::GetFarZ() const
{
    return 0.0f;
}

void KG::Component::FakeGSCascadeCameraComponent::SetViewport(const CameraViewport& viewport)
{
}

void KG::Component::FakeGSCascadeCameraComponent::SetScissorRect(const RECT& rect)
{
}

KG::Component::CameraViewport KG::Component::FakeGSCascadeCameraComponent::GetViewport() const
{
    return CameraViewport();
}

RECT KG::Component::FakeGSCascadeCameraComponent::GetScissorRect() const
{
    return RECT();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCascadeCameraComponent::GetView(size_t index)
{
    return DirectX::XMFLOAT4X4();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCascadeCameraComponent::GetProj(size_t index)
{
    return DirectX::XMFLOAT4X4();
}

std::array<DirectX::XMFLOAT4X4, 4> KG::Component::FakeGSCascadeCameraComponent::GetViewProj()
{
    return std::array<DirectX::XMFLOAT4X4, 4>();
}

DirectX::XMFLOAT4X4 KG::Component::FakeGSCascadeCameraComponent::GetViewProj(size_t viewIndex, size_t projIndex)
{
    return DirectX::XMFLOAT4X4();
}

void KG::Component::FakeGSCascadeCameraComponent::OnPreRender()
{
}

void KG::Component::FakeGSCascadeCameraComponent::InitalizeCascade(KG::Component::ICameraComponent* directionalLightCamera, KG::Component::ILightComponent* light)
{
}

void KG::Component::FakeGSCascadeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeGSCascadeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}
