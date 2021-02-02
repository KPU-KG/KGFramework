#include "pch.h"
#include "LightComponent.h"
#include "KGRenderQueue.h"
#include "ResourceContainer.h"

void KG::Component::LightComponent::SetRenderJob( KG::Renderer::KGRenderJob* renderJob )
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd( this->isVisible );
}

void KG::Component::LightComponent::RegisterTransform( TransformComponent* transform )
{
	this->transform = transform;
}

void KG::Component::LightComponent::SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction )
{
	isDirty = true;
	this->lightType = LightType::DirectionalLight;
	this->light.Direction = Math::Vector3::Normalize( direction );
	this->light.Strength = strength;
	if ( this->directionalLightShader == nullptr )
	{
		this->directionalLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "DirectionalLight"_id ) );
	}
	if ( this->directionalLightGeometry == nullptr )
	{
		this->directionalLightGeometry = KG::Resource::ResourceContainer::GetInstance()->LoadGeometry( Utill::HashString( "lightPlane"_id ) );
	}
	this->currentShader = this->directionalLightShader;
	this->currentGeometry = this->directionalLightGeometry;
}

void KG::Component::LightComponent::SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd )
{
	isDirty = true;
	this->lightType = LightType::PointLight;
	this->light.Strength = strength;
	this->light.FalloffStart = fallOffStart;
	this->light.FalloffEnd = fallOffEnd;
	if ( this->pointLightShader == nullptr )
	{
		this->pointLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "PointLight"_id ) );
	}
	if ( this->pointLightGeometry == nullptr )
	{
		this->pointLightGeometry = KG::Resource::ResourceContainer::GetInstance()->LoadGeometry( Utill::HashString( "sphere"_id ) );
	}
	this->currentShader = this->pointLightShader;
	this->currentGeometry = this->pointLightGeometry;
}

KG::Component::DirectionalLightRef KG::Component::LightComponent::GetDirectionalLightRef()
{
	this->UpdateChanged();
	return DirectionalLightRef( this->light );
}

KG::Component::PointLightRef KG::Component::LightComponent::GetPointLightRef()
{
	this->UpdateChanged();
	return PointLightRef( this->light );
}

void KG::Component::LightComponent::UpdateChanged()
{
	this->isDirty = true;
}

void KG::Component::LightComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->RegisterTransform( gameObject->GetComponent<TransformComponent>() );
	auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->currentShader, this->currentGeometry );
	this->SetRenderJob( job );
}

void KG::Component::LightComponent::OnRender( ID3D12GraphicsCommandList* commadList )
{

}

void KG::Component::LightComponent::SetShadowCasterTextureIndex( UINT index )
{
	this->light.shadowMapIndex = index;
}

void KG::Component::LightComponent::SetShadowMatrix( const DirectX::XMFLOAT4X4 matrix )
{
	//포인트 : 투영행렬
	//나머지 : viewProj
	this->light.shadowMatrix = matrix;
}

void KG::Component::LightComponent::OnPreRender()
{
	if ( this->isDirty )
	{
		this->isDirty = false;
		int updateCount = this->renderJob->GetUpdateCount();
		this->light.Position = this->transform->GetWorldPosition();
		std::memcpy( &this->renderJob->objectBuffer->mappedData[updateCount].light, &this->light, sizeof( this->light ) );
	}
}

void KG::Component::LightComponent::SetVisible( bool visible )
{
	if ( this->isVisible == visible )
		return;
	this->isVisible = visible;
	if ( this->isVisible )
	{
		this->renderJob->OnVisibleAdd();
	}
	else
	{
		this->renderJob->OnVisibleRemove();
	}
}
