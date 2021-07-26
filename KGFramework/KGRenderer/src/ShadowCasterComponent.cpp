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
    this->pointLightCamera->InitializeRenderTexture();
	this->pointLightCamera->SetDefaultRender();
	this->pointLightCamera->SetNearZ( 0.01f );
	this->pointLightCamera->SetFarZ( this->targetLight->GetPointLightRef().FalloffEnd );
	this->pointLightCamera->OnCreate(this->gameObject);
	this->targetLight->SetShadowCasterTextureIndex( this->pointLightCamera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV).HeapIndex );
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
	this->targetLight->SetShadowCasterTextureIndex( this->directionalLightCamera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV).HeapIndex);
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
    this->spotLightCamera->isMainCamera = false;
    this->spotLightCamera->InitializeRenderTexture();
	this->spotLightCamera->SetNearZ( 0.01f );
	this->spotLightCamera->SetFarZ( this->targetLight->GetSpotLightRef().depth );
	this->spotLightCamera->SetFovY( DirectX::XMConvertToDegrees( this->targetLight->GetSpotLightRef().Phi ) );
	this->spotLightCamera->SetAspectRatio( 1.0f );
	this->spotLightCamera->OnCreate( this->gameObject );
	this->targetLight->SetShadowCasterTextureIndex( this->spotLightCamera->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV).HeapIndex );
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
        this->directionalLightCamera->SetMainCamera(static_cast<ICameraComponent*>(this->GetGameObject()->GetScene()->GetMainCamera()));
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

bool KG::Component::ShadowCasterComponent::OnDrawGUI()
{
    if (ImGui::ComponentHeader<KG::Component::ShadowCasterComponent>())
    {

        if (ImGui::Button("View ShadowMap"))
        {
            ImGui::OpenPopup("ShadowMap PopUp");
        }
        if (ImGui::BeginPopup("ShadowMap PopUp"))
        {
            switch (this->GetTargetLightType())
            {
            case LightType::DirectionalLight:
            {
                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::DragFloat("cas 0", this->GetDirectionalLightCamera()->cascadePoint, 0.01f, 0.0f, 1.0f);
                auto handle = this->GetDirectionalLightCamera()->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV, 1).GetGPUHandle();
                ImGui::Image((ImTextureID)handle.ptr, ImVec2(350, 350));
                ImGui::EndGroup();

                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::DragFloat("cas 1", this->GetDirectionalLightCamera()->cascadePoint + 1, 0.01f, 0.0f, 1.0f);
                handle = this->GetDirectionalLightCamera()->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV, 2).GetGPUHandle();
                ImGui::Image((ImTextureID)handle.ptr, ImVec2(350, 350));
                ImGui::EndGroup();

                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::DragFloat("cas 2", this->GetDirectionalLightCamera()->cascadePoint + 2, 0.01f, 0.0f, 1.0f);
                handle = this->GetDirectionalLightCamera()->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV, 3).GetGPUHandle();
                ImGui::Image((ImTextureID)handle.ptr, ImVec2(350, 350));
                ImGui::EndGroup();

                ImGui::SameLine();
                ImGui::BeginGroup();
                ImGui::DragFloat("cas 3", this->GetDirectionalLightCamera()->cascadePoint + 3, 0.01f, 0.0f, 1.0f);
                handle = this->GetDirectionalLightCamera()->GetRenderTexture().depthStencilBuffer.GetDescriptor(DescriptorType::SRV, 4).GetGPUHandle();
                ImGui::Image((ImTextureID)handle.ptr, ImVec2(350, 350));
                ImGui::EndGroup();
            }
                break;
            }
            ImGui::EndPopup();
        }
    }
    return false;
}
