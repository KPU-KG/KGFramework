#include "pch.h"
#include <array>
#include "ShadowCasterComponent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "RenderTexture.h"

using namespace DirectX;

void KG::Component::ShadowCasterComponent::InitializeAsPointLightShadow( )
{
	this->cubeCamera = new GSCubeCameraComponent();
	this->cubeCamera->OnCreate( this->gameObject );
	KG::Renderer::RenderTextureDesc desc;
	desc.useCubeRender = true;
	desc.useGSCubeRender = true;
	desc.useRenderTarget = false;
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
	this->cubeCamera->SetFarZ( this->targetLight->GetPointLightRef().FalloffEnd );
	this->targetLight->SetShadowCasterTextureIndex( this->cubeCamera->GetRenderTexture().depthStencilSRVIndex );
	this->targetLight->SetShadowMatrix( this->cubeCamera->GetProj() );
}

void KG::Component::ShadowCasterComponent::InitializeAsDirectionalLightShadow()
{
	this->camera = new GSCascadeCameraComponent();
	this->camera->OnCreate( this->gameObject );

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
	
	this->camera->InitializeRenderTexture( desc );
	this->camera->SetDefaultRender();
	this->camera->InitalizeCascade( this->mainCamera, this->targetLight );
	this->camera->SetNearZ( 0.01f );
	this->camera->SetFarZ( 500.0f );
	this->targetLight->SetShadowCasterTextureIndex( this->camera->GetRenderTexture().depthStencilSRVIndex );
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
		this->InitializeAsDirectionalLightShadow( );
		break;
	case LightType::PointLight:
		this->InitializeAsPointLightShadow( );
		break;
	case LightType::SpotLight:
		break;
	default:
		break;
	}
}

void KG::Component::ShadowCasterComponent::OnPreRender()
{
	if ( this->isDirectionalLightShadow() )
	{
		this->camera->OnPreRender();
		this->targetLight->SetShadowCascadeMatrix( this->camera->GetViewProj() );
	}
	else if ( this->isPointLightShadow() )
	{
		this->cubeCamera->OnPreRender();
		this->targetLight->SetShadowMatrix( this->cubeCamera->GetProj() );
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

void KG::Component::ShadowCasterComponent::SetTargetCameraCamera( KG::Component::CameraComponent* mainCamera )
{
	this->mainCamera = mainCamera;
}

KG::Renderer::RenderTexture& KG::Component::ShadowCasterComponent::GetRenderTexture()
{
	return this->cubeCamera ? this->cubeCamera->GetRenderTexture() : this->camera->GetRenderTexture();
}

KG::Component::GSCubeCameraComponent* KG::Component::ShadowCasterComponent::GetCubeCamera() const
{
	return this->cubeCamera;
}

KG::Component::GSCascadeCameraComponent* KG::Component::ShadowCasterComponent::GetCamera() const
{
	return this->camera;
}
