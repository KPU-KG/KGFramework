#include "Scene.h"
#include <algorithm>

UINT KG::Core::Scene::InternalGetEmptyObject()
{
	for ( UINT index = 0; index < this->objectPool.size(); ++index )
	{
		if ( this->objectPool[index].first == false )
		{
			this->objectPool[index].first = true;
			return index;
		}
	}
	UINT index = this->objectPool.size();
	this->objectPool.emplace_back();
	return index;
}

KG::Core::GameObject* KG::Core::Scene::GetFrontObject(UINT32 index)
{
	if ( this->frontActivePool.size() < index + 1 )
	{
		return nullptr;
	}
	else
	{
		UINT poolIndex = this->frontActivePool[index];
		return &this->objectPool[poolIndex].second;
	}
}

KG::Core::GameObject* KG::Core::Scene::GetBackObject(UINT32 index)
{
	if ( this->backActivePool.size() < index + 1 )
	{
		return nullptr;
	}
	else
	{
		UINT poolIndex = this->backActivePool[index];
		return &this->objectPool[poolIndex].second;
	}
}

KG::Core::GameObject* KG::Core::Scene::FindFrontObjectWithTag(const KG::Utill::HashString& tag)
{
	for ( auto& i : this->frontActivePool )
	{
		KG::Core::GameObject* obj = this->GetFrontObject(i);
		if ( obj != nullptr && obj->tag == tag )
		{
			return obj;
		}
	}
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::FindBackObjectWithTag(const KG::Utill::HashString& tag)
{
	for ( auto& i : this->backActivePool )
	{
		KG::Core::GameObject* obj = this->GetBackObject(i);
		if ( obj != nullptr && obj->tag == tag )
		{
			return obj;
		}
	}
	return nullptr;
}

UINT32 KG::Core::Scene::ToBackID(UINT32 frontID)
{
	return NULL_OBJECT - frontID;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewObject()
{
	UINT index = this->InternalGetEmptyObject();
	UINT id = this->frontActivePool.size();
	this->frontActivePool.emplace_back(index);
	GameObject* obj = &this->objectPool[index].second;
	obj->SetOwnerScene(this);
	obj->SetInstanceID(id);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewObject(UINT32 instanceID)
{
	UINT index = this->InternalGetEmptyObject();
	UINT backId = ToBackID(instanceID);
	if ( this->backActivePool.size() < backId + 1 )
	{
		this->backActivePool.resize(backId + 1 , NULL_OBJECT);
	}
	this->backActivePool[instanceID] = index;

	GameObject* obj = &this->objectPool[index].second;
	obj->SetOwnerScene(this);
	obj->SetInstanceID(instanceID);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::FindObjectWithTag(const KG::Utill::HashString& tag)
{
	KG::Core::GameObject* result = this->FindFrontObjectWithTag(tag);
	if ( result == nullptr )
		result = this->FindBackObjectWithTag(tag);
	return result;
}

KG::Core::GameObject* KG::Core::Scene::FindObjectWithID(UINT32 instanceID)
{
	UINT32 frontID = instanceID;
	UINT32 backID = ToBackID(frontID);
	KG::Core::GameObject* result = this->GetFrontObject(frontID);
	if ( result == nullptr )
		result = this->GetBackObject(backID);
	return result;
}

void KG::Core::Scene::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
}

void KG::Core::Scene::OnDataSave(tinyxml2::XMLElement* objectElement)
{
}

void KG::Core::Scene::OnDrawGUI()
{
}
