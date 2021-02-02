#include "pch.h"
#include "ShadowCasterComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "RenderTexture.h"

using namespace DirectX;

void KG::Component::ShadowCasterComponent::InitializeAsPointLightShadow( KG::Component::LightComponent* light )
{
	this->cubeCamera = new GSCubeCameraComponent();
	this->cubeCamera->OnCreate( this->gameObject );
	KG::Renderer::RenderTextureDesc desc;
	desc.useCubeRender = true;
	desc.useGSCubeRender = true;
	desc.useDeferredRender = false;
	desc.useDeferredRender = false;
	desc.useDepthStencilBuffer = true;
	desc.uploadSRVDepthBuffer = true;
	desc.uploadSRVRenderTarget = false;
	desc.width = 512;
	desc.height = 512;
	this->cubeCamera->InitializeRenderTexture( desc );
	this->cubeCamera->SetDefaultRender();
	this->cubeCamera->SetNearZ( 0.01f );
	this->cubeCamera->SetFarZ( light->GetPointLightRef().FalloffEnd );
	light->SetShadowCasterTextureIndex( this->cubeCamera->GetRenderTexture().depthStencilSRVIndex );
	//Projection
	light->SetShadowMatrix( this->cubeCamera->GetProj() );
}

void KG::Component::ShadowCasterComponent::InitializeAsDirectionalLightShadow( KG::Component::LightComponent* light )
{
	this->camera = new CameraComponent();
	this->camera->SetDefaultRender();
	this->camera->OnCreate( this->gameObject );
	//TODO
}

void KG::Component::ShadowCasterComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	auto lightComponent = gameObject->GetComponent<LightComponent>();
	//lightComponent->SetCastShadow( true );
	switch ( lightComponent->GetLightType() )
	{
	case LightType::DirectionalLight:
		break;
	case LightType::PointLight:
		this->InitializeAsPointLightShadow( lightComponent );
		break;
	case LightType::SpotLight:
		break;
	default:
		break;
	}
}

void KG::Component::ShadowCasterComponent::OnDestroy()
{
	if ( this->camera )
	{
		this->camera->OnDestroy();
		delete this->camera;
	}
	else if ( this->cubeCamera )
	{
		this->cubeCamera->OnDestroy();
		delete this->cubeCamera;
	}
}

KG::Renderer::RenderTexture& KG::Component::ShadowCasterComponent::GetRenderTexture()
{
	return this->cubeCamera ? this->cubeCamera->GetRenderTexture() : this->camera->GetRenderTexture();
}

KG::Component::GSCubeCameraComponent* KG::Component::ShadowCasterComponent::GetCubeCamera() const
{
	return this->cubeCamera;
}

KG::Component::CameraComponent* KG::Component::ShadowCasterComponent::GetCamera() const
{
	return this->camera;
}
