#include "pch.h"

#include "MaterialComponent.h"
#include "ResourceContainer.h"

#pragma region MaterialComponent

void KG::Component::MaterialComponent::InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	auto [index, shaderId] = inst->LoadMaterial( materialID );
	if ( this->materialIndexs.size() < slotIndex + 1 )
	{
		this->materialIndexs.resize( slotIndex + 1 );
	}
	this->materialIndexs[slotIndex] = index;
	this->InitializeShader( shaderId, slotIndex );
}

void KG::Component::MaterialComponent::InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->shaders.size() < slotIndex + 1 )
	{
		this->shaders.resize( slotIndex + 1 );
	}
	this->shaders[slotIndex] = inst->LoadShader( shaderID );
}

unsigned KG::Component::MaterialComponent::GetMaterialIndex( UINT slotIndex ) const
{
	return this->materialIndexs.size() != 0 ? this->materialIndexs[slotIndex] : 0;
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

#pragma endregion