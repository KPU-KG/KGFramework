#include "pch.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "KGRenderQueue.h"
#include "Transform.h"
#include "MathHelper.h"
#include "GameObject.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "ResourceContainer.h"

using namespace KG::Renderer;

void KG::Component::Render3DComponent::OnRender()
{
}

void KG::Component::Render3DComponent::OnPreRender()
{
	int updateCount = this->renderJob->GetUpdateCount();
	this->renderJob->objectBuffer->mappedData[updateCount].world = Math::Matrix4x4::Transpose( this->transform->GetGlobalWorldMatrix() );
	this->renderJob->objectBuffer->mappedData[updateCount].materialIndex = this->material->GetMaterialIndex();
}

void KG::Component::Render3DComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->RegisterTransform( gameObject->GetComponent<TransformComponent>() );
	this->RegisterMaterial( gameObject->GetComponent<MaterialComponent>() );
	this->RegisterGeometry( gameObject->GetComponent<GeometryComponent>() );
	auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->material->shaders, this->geometry->geometry );
	this->SetRenderJob( job );
}

void KG::Component::Render3DComponent::SetVisible( bool visible )
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

void KG::Component::Render3DComponent::SetRenderJob( KG::Renderer::KGRenderJob* renderJob )
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd( this->isVisible );
}

void KG::Component::Render3DComponent::RegisterTransform( TransformComponent* transform )
{
	this->transform = transform;
}

void KG::Component::Render3DComponent::RegisterMaterial( MaterialComponent* material )
{
	this->material = material;
}

void KG::Component::Render3DComponent::RegisterGeometry( GeometryComponent* geometry )
{
	this->geometry = geometry;
}

void KG::Component::MaterialComponent::InitializeShader( const KG::Utill::HashString& shaderID )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	this->shaders = inst->LoadShader( shaderID );
}

unsigned KG::Component::MaterialComponent::GetMaterialIndex() const
{
	return 0;
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

void KG::Component::GeometryComponent::InitializeGeometry( const KG::Utill::HashString& shaderID )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	this->geometry = inst->LoadGeometry( shaderID );
}

