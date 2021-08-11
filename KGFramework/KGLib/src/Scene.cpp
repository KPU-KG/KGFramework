#include <functional>
#include <algorithm>
#include <vector>
#include "Scene.h"
#include "Transform.h"
#include "MaterialMatch.h"

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
	this->objectPool[index].first = true;
	return index;
}

KG::Core::GameObject* KG::Core::Scene::GetIndexObject(UINT32 index) const
{
	if ( this->activePool.size() < index + 1 )
	{
		return nullptr;
	}
	else
	{
		UINT poolIndex = this->activePool[index];
		return poolIndex == NULL_OBJECT ? nullptr : const_cast<KG::Core::GameObject*>(&(this->objectPool.at(poolIndex).second));
	}
}

UINT32 KG::Core::Scene::GetEmptyID()
{
	for ( size_t i = 0; i < this->activePool.size(); i++ )
	{
		if ( this->activePool[i] == NULL_OBJECT )
		{
			return i;
		}
	}
	return this->activePool.size();
}

KG::Core::Scene::Scene()
	:skyBoxIdProp("SkyBoxId", this->skyBoxId)
{
}

KG::Core::Scene::~Scene()
{
	DebugNormalMessage("SceneDestroy");
}

void KG::Core::Scene::SetComponentProvider(KG::Component::ComponentProvider* componentProvider)
{
	this->componentProvider = componentProvider;
}

KG::Component::ComponentProvider* KG::Core::Scene::GetComponentProvider() const
{
	return this->componentProvider;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewObject()
{
	UINT index = this->InternalGetEmptyObject();
	UINT id = this->activePool.size();
	this->activePool.emplace_back(index);
	GameObject* obj = &this->objectPool[index].second;
	memset(obj, 0, sizeof(GameObject));
	new(obj) GameObject();
	obj->SetOwnerScene(this);
	obj->SetInstanceID(id);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewObject(UINT32 instanceID)
{
	UINT index = this->InternalGetEmptyObject();
	if ( this->activePool.size() < instanceID + 1 )
	{
		this->activePool.resize(instanceID + 1, NULL_OBJECT);
	}
	this->activePool[instanceID] = index;

	GameObject* obj = &this->objectPool[index].second;
	memset(obj, 0, sizeof(GameObject));
	new(obj) GameObject();
	obj->SetOwnerScene(this);
	obj->SetInstanceID(instanceID);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewTransformObject()
{
	auto* obj = this->CreateNewObject();
	this->componentProvider->AddComponentToObject(KG::Component::ComponentID <KG::Component::TransformComponent>::id(), obj);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::CreateNewTransformObject(UINT32 instanceID)
{
	auto* obj = this->CreateNewObject(instanceID);
	this->componentProvider->AddComponentToObject(KG::Component::ComponentID <KG::Component::TransformComponent>::id(), obj);
	return obj;
}

KG::Core::GameObject* KG::Core::Scene::CreateCopyObject(const KG::Core::GameObject* sourceObject)
{
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::CreatePrefabObjcet(const KG::Utill::HashString& prefabId)
{
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::CreatePrefabObjcet(const KG::Utill::HashString& prefabId, UINT32 instanceID)
{
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::FindObjectWithTag(const KG::Utill::HashString& tag) const
{
	for ( size_t i = 0; i < this->activePool.size(); i++ )
	{
		auto* object = this->GetIndexObject(i);
		if ( object && object->tag == tag )
		{
			return object;
		}
	}
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::FindObjectWithID(UINT32 instanceID) const
{
	return this->GetIndexObject(instanceID);
}

KG::Core::GameObject* KG::Core::Scene::GetRootNode() const
{
    return this->rootNode;
}

UINT KG::Core::Scene::GetObjectCount() const
{
	return this->activePool.size() - std::count(this->activePool.begin(), this->activePool.end(), NULL_OBJECT);
}

void KG::Core::Scene::LoadScene(const std::string& path)
{
	this->sourceDocument.LoadFile(path.c_str());
	tinyxml2::XMLElement* sceneElement = this->sourceDocument.FirstChildElement("Scene");

	this->OnDataLoad(sceneElement);
}

void KG::Core::Scene::SaveCurrentScene(const std::string& path)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLDeclaration* dec1 = doc.NewDeclaration();
	tinyxml2::XMLElement* objectElement = doc.NewElement("Scene");
	this->OnDataSave(objectElement);
	doc.LinkEndChild(dec1);
	doc.LinkEndChild(objectElement);
	auto e = doc.SaveFile(path.c_str());
}

KG::Component::IComponent* KG::Core::Scene::SetMainCamera(KG::Component::IComponent* mainCamera)
{
	auto* temp = this->mainCamera;
	this->mainCamera = mainCamera;
	return temp;
}

KG::Component::IComponent* KG::Core::Scene::GetMainCamera() const
{
	return this->mainCamera;
}

DirectX::XMFLOAT4X4 KG::Core::Scene::GetMainCameraView() const
{
	return this->getViewFunc(this->GetMainCamera());
}

DirectX::XMFLOAT4X4 KG::Core::Scene::GetMainCameraProj() const
{
	return this->getProjFunc(this->GetMainCamera());
}

void KG::Core::Scene::AddSceneCameraObjectCreator(SceneCameraCreator&& creator)
{
	this->sceneCameraCreator = creator;
}

void KG::Core::Scene::AddSkyBoxObjectCreator(SkyBoxCreator&& creator)
{
	this->skyBoxCreator = creator;
}

void KG::Core::Scene::OnDataLoad(tinyxml2::XMLElement* sceneElement)
{
	this->skyBoxIdProp.OnDataLoad(sceneElement);
	this->skyBoxSetter(this->skyBoxId);

	auto* objectElement = sceneElement->FirstChildElement("GameObject");
	while ( objectElement )
	{
		auto nameStr = std::string(objectElement->Name());
		UINT32 id = objectElement->UnsignedAttribute("instanceId");

		if ( nameStr == "GameObject" )
		{
			this->rootNode->OnDataLoad(objectElement);
		}
		else if ( nameStr == "Prefab" )
		{
			UINT32 prefabId = objectElement->UnsignedAttribute("prefab_hash_id");
			auto* obj = this->CreatePrefabObjcet(KG::Utill::HashString(prefabId), id);
		}
		objectElement = objectElement->NextSiblingElement();
	}
}

void KG::Core::Scene::OnDataSave(tinyxml2::XMLElement* sceneElement)
{
	UINT count = 0;
	this->skyBoxIdProp.OnDataSave(sceneElement);
	this->rootNode->OnDataSave(sceneElement);
	//for ( UINT32 i : this->frontActivePool )
	//{
	//	if ( i != NULL_OBJECT )
	//	{
	//		this->GetFrontObject(i)->OnDataSave(sceneElement);
	//		count++;
	//	}
	//}
}

void KG::Core::Scene::AddObjectPreset(std::string name, PresetObjectCreator&& creator)
{
	this->objectPresetName.emplace_back(name);
	this->objectPresetModel.emplace_back(nullptr);
	this->objectPresetFunc.emplace_back(creator);
}

void KG::Core::Scene::AddModelPreset(std::string name, PresetModelCreator&& modelCreator, PresetObjectCreator&& objCreator)
{
	this->objectPresetName.emplace_back(name);
	this->objectPresetModel.emplace_back(modelCreator);
	this->objectPresetFunc.emplace_back(objCreator);
}


void KG::Core::Scene::AddNetworkCreator(const KG::Utill::HashString& hashId, NetworkObjectCreator&& creator)
{
	this->networkPresetFunc.emplace(hashId, creator);
}

void KG::Core::Scene::AddSkySetter(SkyBoxSetter&& setter)
{
	this->skyBoxSetter = setter;
}

void KG::Core::Scene::AddModelCreator(ModelCreator&& creator)
{
	this->modelCreator = creator;
}

void KG::Core::Scene::AddCameraMatrixGetter(GetMatrixFunc&& view, GetMatrixFunc&& proj)
{
	this->getViewFunc = view;
	this->getProjFunc = proj;
}

KG::Core::GameObject* KG::Core::Scene::CallPreset(const std::string& name)
{
	KG::Core::GameObject* obj = nullptr;;
	for ( size_t i = 0; i < this->objectPresetName.size(); i++ )
	{
		if ( this->objectPresetName[i] == name )
		{
			if ( objectPresetModel[i] != nullptr )
			{
				auto [modelId, materialMach] = this->objectPresetModel[i]();
				obj =  this->modelCreator(modelId, *this, materialMach);
			}
			else
			{
				obj =  this->CreateNewObject();
			}
			objectPresetFunc[i](*obj);
			obj->tag = KG::Utill::HashString(objectPresetName[i]);
			return obj;
		}
	}
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::CallPreset(const KG::Utill::HashString& hashid)
{
	KG::Core::GameObject* obj = nullptr;;
	for ( size_t i = 0; i < this->objectPresetName.size(); i++ )
	{
		if ( KG::Utill::HashString(this->objectPresetName[i]) == hashid )
		{
			if ( objectPresetModel[i] != nullptr )
			{
				auto [modelId, materialMach] = this->objectPresetModel[i]();
				obj = this->modelCreator(modelId, *this, materialMach);
			}
			else
			{
				obj = this->CreateNewTransformObject();
			}
			objectPresetFunc[i](*obj);
			obj->tag = KG::Utill::HashString(objectPresetName[i]);
			return obj;
		}
	}
	return nullptr;
}

KG::Component::IComponent* KG::Core::Scene::CallNetworkCreator(const KG::Utill::HashString& hashid)
{
	auto* node =this->CallPreset(hashid);
	auto it = this->networkPresetFunc.find(hashid);
	if ( it == this->networkPresetFunc.end() )
		return nullptr;
	auto* comp = it->second(*node);
	return comp;
}


void KG::Core::Scene::AddSceneComponent(const KG::Utill::HashString& hashid, KG::Component::IComponent* component)
{
	this->rootNode->AddComponentWithID(hashid, component);
}

void KG::Core::Scene::Clear()
{
	this->rootNode->DestroyAllChild();
	this->InternalDeleteQueuing();
	this->mainCamera = nullptr;
    this->AddDeleteQueue(this->rootNode);
    this->InternalDeleteQueuing();
    this->rootNode = nullptr;
	this->InitializeRoot();
}

void KG::Core::Scene::InitializeRoot()
{
    this->rootNode = this->CallPreset("EmptyObject"_id);
	this->rootNode->tag = KG::Utill::HashString("rootNode");
	this->rootNode->SetOwnerScene(this);
}

void KG::Core::Scene::InternalDeleteQueuing()
{
	while ( !this->deleteQueue.empty() )
	{
		auto* deleteObj = this->deleteQueue.front();
		deleteObj->DeleteAllComponent();
		this->deleteQueue.pop();
	}
}

void KG::Core::Scene::AddDeleteQueue(KG::Core::GameObject* obj)
{
	this->deleteQueue.push(obj);
}

void KG::Core::Scene::Update(float elaspedTime)
{

}
void KG::Core::Scene::PostUpdate(float elaspedTime)
{
	this->InternalDeleteQueuing();
}