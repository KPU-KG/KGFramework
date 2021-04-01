#include "pch.h"
#include "LightComponent.h"
#include "KGRenderQueue.h"
#include "ResourceContainer.h"

void KG::Component::LightComponent::SetRenderJob(KG::Renderer::KGRenderJob* renderJob)
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd(this->isVisible);
}

void KG::Component::LightComponent::RegisterTransform(TransformComponent* transform)
{
	this->transform = transform;
}

void KG::Component::LightComponent::SetDirectionalLight(const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction)
{
	isDirty = true;
	this->lightType = LightType::DirectionalLight;
	this->light.Direction = Math::Vector3::Normalize(direction);
	this->light.Strength = strength;

}


void KG::Component::LightComponent::SetPointLight(const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd)
{
	isDirty = true;
	this->lightType = LightType::PointLight;
	this->light.Strength = strength;
	this->light.FalloffStart = fallOffStart;
	this->light.FalloffEnd = fallOffEnd;
}

void KG::Component::LightComponent::SetSpotLight(const DirectX::XMFLOAT3& strength, float depth, float Phi, float Theta, float fallOff)
{
	isDirty = true;
	this->lightType = LightType::SpotLight;
	this->light.Theta = Theta;
	this->light.Phi = Phi;
	this->light.Strength = strength;
	this->light.FalloffStart = depth;
	this->light.FalloffEnd = fallOff;
}

KG::Component::LightComponent::LightComponent()
	:
	lightTypeProp(
		"LightType", this->lightType,
		{
			{LightType::DirectionalLight, "DirectionalLight"},
			{LightType::PointLight, "PointLight"},
			{LightType::SpotLight, "SpotLight"}
		},
		true),
	strengthProp("Strength", this->light.Strength),
	directionProp("Direction", this->light.Direction),
	fallOffStartProp("FallOffStart", this->light.FalloffStart),
	fallOffEndProp("FallOffEnd", this->light.FalloffEnd),
	depthProp("Depth", this->light.FalloffStart),
	phiProp("Phi", this->light.Phi),
	thetaProp("Theta", this->light.Theta),
	fallOffProp("FallOff", this->light.FalloffEnd)
{
}

void KG::Component::LightComponent::SetLightPower(float lightPower)
{
	this->light.lightPower = lightPower;
}

KG::Component::DirectionalLightRef KG::Component::LightComponent::GetDirectionalLightRef()
{
	this->UpdateChanged();
	return DirectionalLightRef(this->light);
}

KG::Component::PointLightRef KG::Component::LightComponent::GetPointLightRef()
{
	this->UpdateChanged();
	return PointLightRef(this->light);
}

KG::Component::SpotLightRef KG::Component::LightComponent::GetSpotLightRef()
{
	return SpotLightRef(this->light);
}

void KG::Component::LightComponent::UpdateChanged()
{
	this->isDirty = true;
}

void KG::Component::LightComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	IRenderComponent::OnCreate(gameObject);
	memset(&this->shadow, 0, sizeof(this->shadow));
	this->RegisterTransform(gameObject->GetComponent<TransformComponent>());
	switch ( this->lightType )
	{
		case LightType::DirectionalLight:
			this->InitializeDirectionalLight();
			break;
		case LightType::PointLight:
			this->InitializePointLight();
			break;
		case LightType::SpotLight:
			this->InitializeSpotLight();
			break;
	}
	auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->currentShader, this->currentGeometry);
	this->SetRenderJob(job);
}

void KG::Component::LightComponent::InitializeDirectionalLight()
{
	if ( this->directionalLightShader == nullptr )
	{
		this->directionalLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader(Utill::HashString("DirectionalLight"_id));
	}
	if ( this->directionalLightGeometry == nullptr )
	{
		this->directionalLightGeometry = KG::Resource::ResourceContainer::GetInstance()->LoadGeometry(Utill::HashString("lightPlane"_id));
	}
	this->currentShader = this->directionalLightShader;
	this->currentGeometry = this->directionalLightGeometry;
}

void KG::Component::LightComponent::InitializePointLight()
{
	if ( this->pointLightShader == nullptr )
	{
		this->pointLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader(Utill::HashString("PointLight"_id));
	}
	if ( this->pointLightGeometry == nullptr )
	{
		this->pointLightGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, 2);
	}
	this->currentShader = this->pointLightShader;
	this->currentGeometry = this->pointLightGeometry;
}

void KG::Component::LightComponent::InitializeSpotLight()
{
	if ( this->spotLightShader == nullptr )
	{
		this->spotLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader(Utill::HashString("SpotLight"_id));
	}
	if ( this->spotLightGeometry == nullptr )
	{
		this->spotLightGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST, 1);
	}
	this->currentShader = this->spotLightShader;
	this->currentGeometry = this->spotLightGeometry;
}

void KG::Component::LightComponent::OnRender(ID3D12GraphicsCommandList* commadList)
{

}

void KG::Component::LightComponent::SetShadowCasterTextureIndex(UINT index)
{
	this->UpdateChanged();
	this->shadow.shadowMapIndex[0] = index;
}


void KG::Component::LightComponent::SetShadowMatrix(const DirectX::XMFLOAT4X4 matrix)
{
	//포인트 : 투영행렬
	//나머지 : viewProj
	this->UpdateChanged();
	this->shadow.shadowMatrix[0] = matrix;
}

void KG::Component::LightComponent::SetShadowCascadeMatrix(const std::array<DirectX::XMFLOAT4X4, 4>& cascades)
{
	this->UpdateChanged();
	for ( size_t i = 0; i < 4; i++ )
	{
		this->shadow.shadowMatrix[i] = cascades[i];
	}
}

void KG::Component::LightComponent::OnPreRender()
{
	if ( this->isDirty || this->lightType == LightType::SpotLight )
	{
		this->isDirty = false;
		int updateCount = this->renderJob->GetUpdateCount();
		this->light.Position = this->transform->GetWorldPosition();
		std::memcpy(&this->renderJob->objectBuffer->mappedData[updateCount].light, &this->light, sizeof(this->light));
		if ( this->renderJob->shadowLightBuffer != nullptr )
			std::memcpy(&this->renderJob->shadowLightBuffer->mappedData[updateCount].shadow, &this->shadow, sizeof(this->shadow));
		if ( this->lightType == LightType::SpotLight )
		{
			this->renderJob->objectBuffer->mappedData[updateCount].light.Direction = (this->transform->GetWorldLook());
			this->renderJob->objectBuffer->mappedData[updateCount].light.Up = (this->transform->GetWorldUp());
		}
	}
}

void KG::Component::LightComponent::SetVisible(bool visible)
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

void KG::Component::LightComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->lightTypeProp.OnDataLoad(componentElement);
	this->strengthProp.OnDataLoad(componentElement);
	switch ( this->lightType )
	{
		case LightType::DirectionalLight:
			this->directionProp.OnDataLoad(componentElement);
			break;
		case LightType::PointLight:
			this->fallOffStartProp.OnDataLoad(componentElement);
			this->fallOffEndProp.OnDataLoad(componentElement);
			break;
		case LightType::SpotLight:
			this->depthProp.OnDataLoad(componentElement);
			this->phiProp.OnDataLoad(componentElement);
			this->thetaProp.OnDataLoad(componentElement);
			this->fallOffProp.OnDataLoad(componentElement);
			break;
	}
}

void KG::Component::LightComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::LightComponent);
	this->lightTypeProp.OnDataSave(componentElement);
	this->strengthProp.OnDataSave(componentElement);
	switch ( this->lightType )
	{
		case LightType::DirectionalLight:
			this->directionProp.OnDataSave(componentElement);
			break;
		case LightType::PointLight:
			this->fallOffStartProp.OnDataSave(componentElement);
			this->fallOffEndProp.OnDataSave(componentElement);
			break;
		case LightType::SpotLight:
			this->depthProp.OnDataSave(componentElement);
			this->phiProp.OnDataSave(componentElement);
			this->thetaProp.OnDataSave(componentElement);
			this->fallOffProp.OnDataSave(componentElement);
			break;
	}
}

bool KG::Component::LightComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<LightComponent>() )
	{
		bool flag = false;
		this->lightTypeProp.OnDrawGUI();
		flag |= this->strengthProp.OnDrawGUI();
		switch ( this->lightType )
		{
			case LightType::DirectionalLight:
				flag |= this->directionProp.OnDrawGUI();
				break;
			case LightType::PointLight:
				flag |= this->fallOffStartProp.OnDrawGUI();
				flag |= this->fallOffEndProp.OnDrawGUI();
				break;
			case LightType::SpotLight:
				flag |= this->depthProp.OnDrawGUI();
				flag |= this->phiProp.OnDrawGUI();
				flag |= this->thetaProp.OnDrawGUI();
				flag |= this->fallOffProp.OnDrawGUI();
				break;
		}
		if ( flag )
		{
			this->UpdateChanged();
		}
	}
	return false;
}
