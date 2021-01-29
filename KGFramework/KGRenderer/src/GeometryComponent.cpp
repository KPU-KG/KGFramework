#include "pch.h"
#include "GeometryComponent.h"
#include "ResourceContainer.h"

void KG::Component::GeometryComponent::InitializeGeometry( const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->geometrys.size() < slotIndex + 1 )
	{
		this->geometrys.resize( slotIndex + 1 );
	}
	this->geometrys[slotIndex] = inst->LoadGeometry( geometryID, subMeshIndex );
}

bool KG::Component::GeometryComponent::HasBone() const
{
	bool result = false;
	for ( auto* i : geometrys )
	{
		if ( i->HasBone() )
			return true;
	}
	return false;
}
