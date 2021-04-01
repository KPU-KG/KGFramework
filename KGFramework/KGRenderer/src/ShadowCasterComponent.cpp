#include "pch.h"
#include <array>
#include "Scene.h"
#include "ShadowCasterComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "RenderTexture.h"

using namespace DirectX;

void KG::Component::ShadowCasterComponent::InitializeAsPointLightShadow()
{
	this->pointLightCamera = new GSCubeCameraComponent();
	KG::Renderer::RenderTextureDesc desc;
	desc.useCubeRender = true;
	desc.useGSCubeRender = true;
	desc.useRenderTarget = false;
	desc.useDeferredRender = false;
	desc.useDeferredRender = false;
	desc.useDepthStencilBuffer = true;
	desc.uploadSRVDepthBuffer = true;
	desc.uploadSRVRenderTarget = false;
	desc.width = 1024;
	desc.height = 1024;
	this->pointLightCamera->renderTextureDesc = desc;
	this->pointLightCamera->SetDefaultRender();
	this->pointLightCamera->SetNearZ( 0.01f );
	this->pointLightCamera->SetFarZ( this->targetLight->GetPointLightRef().FalloffEnd );
	this->pointLightCamera->OnCreate(this->gameObject);
	this->targetLight->SetShadowCasterTextureIndex( this->pointLightCamera->GetRenderTexture().depthStencilSRVIndex );
	this->targetLight->SetShadowMatrix( this->pointLightCamera->GetProj() );
}

void KG::Component::ShadowCasterComponent::InitializeAsDirectionalLightShadow()
{
	this->directionalLightCamera = new GSCascadeCameraComponent();

	KG::Renderer::RenderTextureDesc desc;
	desc.useCubeRender = false;
	desc.useGSCubeRender = false;
	desc.useRenderTarget = false;
	desc.useGSArrayRender = true;
	desc.useDeferredRender = false;
	desc.useDeferredRender = false;
	desc.useDepthStencilBuffer = true;
	desc.uploadSRVDepthBuffer = true;
	desc.uploadSRVRenderTarget = false;
	desc.width = 2048;
	desc.height = 2048;
	//desc.width = 4096;
	//desc.height = 4096;
	desc.length = 4;
	this->directionalLightCamera->renderTextureDesc = desc;
	this->directionalLightCamera->InitalizeCascade( static_cast<CameraComponent*>(this->gameObject->GetScene()->GetMainCamera()), this->targetLight );
	this->directionalLightCamera->SetNearZ( 0.01f );
	this->directionalLightCamera->SetFarZ( 500.0f );
	this->directionalLightCamera->OnCreate(this->gameObject);
	this->directionalLightCamera->SetDefaultRender();
	this->targetLight->SetShadowCasterTextureIndex( this->directionalLightCamera->GetRenderTexture().depthStencilSRVIndex );
}

void KG::Component::ShadowCasterComponent::InitializeAsSpotLightShadow()
{
	this->spotLightCamera = new CameraComponent();

	KG::Renderer::RenderTextureDesc desc;
	desc.useCubeRender = false;
	desc.useGSCubeRender = false;
	desc.useRenderTarget = false;
	desc.useGSArrayRender = false;
	desc.useDeferredRender = false;
	desc.useDepthStencilBuffer = true;
	desc.uploadSRVDepthBuffer = true;
	desc.uploadSRVRenderTarget = false;
	desc.width = 4096;
	desc.height = 4096;
	desc.length = 1;
	this->spotLightCamera->renderTextureDesc = desc;
	this->spotLightCamera->SetNearZ( 0.01f );
	this->spotLightCamera->SetFarZ( this->targetLight->GetSpotLightRef().depth );
	this->spotLightCamera->SetFovY( DirectX::XMConvertToDegrees( this->targetLight->GetSpotLightRef().Phi ) );
	this->spotLightCamera->SetAspectRatio( 1.0f );
	this->spotLightCamera->OnCreate( this->gameObject );
	this->targetLight->SetShadowCasterTextureIndex( this->spotLightCamera->GetRenderTexture().depthStencilSRVIndex );
}

void KG::Component::ShadowCasterComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	auto lightComponent = gameObject->GetComponent<LightComponent>();
	this->targetLight = lightComponent;
	//lightComponent->SetCastShadow( true );

	switch ( lightComponent->GetLightType() )
	{
	case LightType::DirectionalLight:
		this->InitializeAsDirectionalLightShadow();
		break;
	case LightType::PointLight:
		this->InitializeAsPointLightShadow();
		break;
	case LightType::SpotLight:
		this->InitializeAsSpotLightShadow();
		break;
	}
}

void KG::Component::ShadowCasterComponent::OnPreRender()
{
	switch ( this->targetLight->GetLightType() )
	{
	case KG::Component::LightType::DirectionalLight:
		this->directionalLightCamera->OnPreRender();
		this->targetLight->SetShadowCascadeMatrix( this->directionalLightCamera->GetViewProj() );
		return;
	case KG::Component::LightType::PointLight:
		this->pointLightCamera->OnPreRender();
		this->targetLight->SetShadowMatrix( this->pointLightCamera->GetProj() );
		return;
	case KG::Component::LightType::SpotLight:
		this->spotLightCamera->OnPreRender();
		this->targetLight->SetShadowMatrix( this->spotLightCamera->GetViewProj() );
		return;
	}
}

void KG::Component::ShadowCasterComponent::OnDestroy()
{
	switch ( this->targetLight->GetLightType() )
	{
	case KG::Component::LightType::DirectionalLight:
		this->directionalLightCamera->OnDestroy();
		delete this->directionalLightCamera;
		return;
	case KG::Component::LightType::PointLight:
		this->pointLightCamera->OnDestroy();
		delete this->pointLightCamera;
		return;
	case KG::Component::LightType::SpotLight:
		this->spotLightCamera->OnDestroy();
		delete this->spotLightCamera;
		return;
	}

}

KG::Renderer::RenderTexture& KG::Component::ShadowCasterComponent::GetRenderTexture()
{
	switch ( this->targetLight->GetLightType() )
	{
	case KG::Component::LightType::DirectionalLight:
		return this->directionalLightCamera->GetRenderTexture();
	case KG::Component::LightType::PointLight:
		return this->pointLightCamera->GetRenderTexture();
	case KG::Component::LightType::SpotLight:
		return this->spotLightCamera->GetRenderTexture();
	}
}

KG::Component::GSCubeCameraComponent* KG::Component::ShadowCasterComponent::GetPointLightCamera() const
{
	return this->pointLightCamera;
}

KG::Component::GSCascadeCameraComponent* KG::Component::ShadowCasterComponent::GetDirectionalLightCamera() const
{
	return this->directionalLightCamera;
}

KG::Component::CameraComponent* KG::Component::ShadowCasterComponent::GetSpotLightCamera() const
{
	return this->spotLightCamera;
}

KG::Component::LightType KG::Component::ShadowCasterComponent::GetTargetLightType() const
{
	return this->targetLight->GetLightType();
}

void KG::Component::ShadowCasterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::ShadowCasterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::ShadowCasterComponent);
}
