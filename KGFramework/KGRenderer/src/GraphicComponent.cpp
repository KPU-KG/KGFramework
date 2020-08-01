#include "pch.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "KGRenderQueue.h"
#include "Transform.h"
#include "MathHelper.h"
#include "GameObject.h"

using namespace KG::Renderer;

void KG::Component::Render3DComponent::OnRender()
{
}

void KG::Component::Render3DComponent::OnPreRender()
{
	int updateCount = this->renderJob->GetUpdateCount();
	this->renderJob->objectBuffer->mappedData[updateCount].world = Math::Matrix4x4::Transpose(this->transform->GetGlobalWorldMatrix());
}

void KG::Component::Render3DComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	IRenderComponent::OnCreate(gameObject);
	this->RegisterTransform(gameObject->components.GetComponent<TransformComponent>());
}

void KG::Component::Render3DComponent::SetVisible(bool visible)
{
	if (this->isVisible == visible)
		return;
	this->isVisible = visible;
	if (this->isVisible)
	{
		this->renderJob->OnVisibleAdd();
	}
	else 
	{
		this->renderJob->OnVisibleRemove();
	}
}

void KG::Component::Render3DComponent::SetRenderJob(KG::Renderer::KGRenderJob* renderJob)
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd(this->isVisible);
}

void KG::Component::Render3DComponent::RegisterTransform(TransformComponent* transform)
{
	this->transform = transform;
}


REGISTER_COMPONENT_ID(KG::Component::IRenderComponent);
REGISTER_COMPONENT_ID(KG::Component::CameraComponent);
REGISTER_COMPONENT_ID(KG::Component::Render3DComponent);
REGISTER_COMPONENT_ID(KG::Component::LightComponent);
