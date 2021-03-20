#include "Scene.h"
#include "Transform.h"
#include "MaterialMatch.h"
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
		return const_cast<KG::Core::GameObject*>(&(this->objectPool.at(poolIndex).second));
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
			return this->GetIndexObject(i);
		}
	}
	return nullptr;
}

KG::Core::GameObject* KG::Core::Scene::FindObjectWithID(UINT32 instanceID) const
{
	return this->GetIndexObject(instanceID);
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
			this->rootNode.OnDataLoad(objectElement);
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
	this->rootNode.OnDataSave(sceneElement);
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
	this->objectPresetName.push_back(name);
	this->objectPresetFunc.push_back(creator);
}

void KG::Core::Scene::AddSkySetter(SkyBoxSetter&& setter)
{
	this->skyBoxSetter = setter;
}

void KG::Core::Scene::AddModelCreator(ModelCreator&& creator)
{
	this->modelCreator = creator;
}

void KG::Core::Scene::InitializeRoot()
{
	this->objectPresetFunc[0](this->rootNode);
	this->rootNode.tag = KG::Utill::HashString("rootNode");
	this->rootNode.SetOwnerScene(this);
}

void KG::Core::Scene::DrawObjectTree(KG::Core::GameObject* node, KG::Core::GameObject*& focused, int& count)
{
	bool selected = focused == node;
	if ( !node )
	{
		return;
	}
	ImGui::PushID(count);
	count++;
	bool opend = ImGui::TreeNodeEx(node->tag.srcString.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | (!selected ? 0 : ImGuiTreeNodeFlags_Selected));
	//Drag And Drop
	{
		if ( ImGui::BeginDragDropTarget() )
		{
			auto* payLoad = ImGui::AcceptDragDropPayload("_GameObject");
			if ( payLoad )
			{
				KG::Core::GameObject* dragSource = *static_cast<KG::Core::GameObject**>(payLoad->Data);
				if ( dragSource && dragSource != node && dragSource->GetTransform() && node->GetTransform() )
				{
					dragSource->GetTransform()->ExtractThisNode();
					node->GetTransform()->AddChild(dragSource->GetTransform());
				}
			}
			ImGui::EndDragDropTarget();
		}
		if ( ImGui::BeginDragDropSource() )
		{
			ImGui::Text(node->tag.srcString.c_str());
			ImGui::SetDragDropPayload("_GameObject", &node, sizeof(KG::Core::GameObject*));
			ImGui::EndDragDropSource();
		}
	}
	if ( opend )
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImU32 col = ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
		if ( ImGui::IsItemClicked() )
		{
			focused = node;
		}

		if ( node->GetChild() )
		{
			DrawObjectTree(node->GetChild(), focused, count);
		}
		ImGui::TreePop();
	}
	else if ( ImGui::IsItemClicked() )
	{
		focused = node;
	}

	DrawObjectTree(node->GetSibling(), focused, count);
	ImGui::PopID();
}

bool KG::Core::Scene::OnDrawGUI()
{
	static KG::Core::GameObject* currentFocusedObject = nullptr;
	static constexpr int sceneInfoSize = 250;
	static constexpr int inspectorSize = 400;
	static bool isSelectedSave = false;
	static ImGuiWindowFlags flag = ImGuiWindowFlags_MenuBar;
	static ImGuiTreeNodeFlags treeNodeFlag = ImGuiTreeNodeFlags_DefaultOpen;
	static KG::Resource::MaterialMatch materialMatchCache;
	static KG::Utill::HashString modelHash;
	auto viewportSize = ImGui::GetMainViewport()->Size;
	ImGui::ShowDemoWindow();
	ImGui::SetNextWindowSize(ImVec2(sceneInfoSize, viewportSize.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if ( ImGui::Begin("Scene Info", &this->isShowHierarchy, flag) )
	{
		if ( ImGui::BeginMenuBar() )
		{
			if ( ImGui::BeginMenu("File") )
			{
				if ( ImGui::MenuItem("Save") )
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSceneSaveKey", " Choose a File", ".xml"
						, ImGui::GetCurrentShortPath("Resource\\Scenes\\"), "SceneData", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
				if ( ImGui::MenuItem("Load") )
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSceneOpenKey", " Choose a File", ".xml",
						ImGui::GetCurrentShortPath("Resource\\Scenes\\"), "SceneData", 1, nullptr, ImGuiFileDialogFlags_None);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if ( ImGui::CollapsingHeader("Info", treeNodeFlag) )
		{
			ImGui::BulletText("Current Object Count : %d", this->GetObjectCount());
		}
		if ( ImGui::CollapsingHeader("Add Scene Object", treeNodeFlag) )
		{
			if ( ImGui::TreeNodeEx("System Object", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow) )
			{
				if ( ImGui::Button("Add Scene Camera Object") )
				{
					auto* obj = this->CreateNewObject();
					sceneCameraCreator(*obj);
					this->rootNode.GetTransform()->AddChild(obj->GetTransform());
				}
				if ( ImGui::Button("Add SkyBox Object") )
				{
					auto* obj = this->CreateNewObject();
					skyBoxSetter(this->skyBoxId);
					skyBoxCreator(*obj, this->skyBoxId);
					this->rootNode.GetTransform()->AddChild(obj->GetTransform());
				}
				ImGui::SetNextItemWidth(160);
				ImGui::SameLine();
				this->skyBoxIdProp.OnDrawGUI();
				ImGui::TreePop();
			}
			if ( ImGui::TreeNodeEx("Normal Object", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow) )
			{
				if ( ImGui::Button("Add Empty Object") )
				{
					auto* obj = this->CreateNewObject();
					objectPresetFunc[0](*obj);
					obj->tag = KG::Utill::HashString(objectPresetName[0]);
					this->rootNode.GetTransform()->AddChild(obj->GetTransform());
				}
				if ( ImGui::Button("Add File Object") )
				{
					ImGuiFileDialog::Instance()->OpenDialog("ChooseObjectOpenKey", " Choose a File", ".xml",
						ImGui::GetCurrentShortPath("Resource\\Objects\\"), "", 1, nullptr, ImGuiFileDialogFlags_None);
				}


				ImGui::SetNextItemWidth(160);
				ImGui::Combo("Preset", &this->currentSelectedPreset, &ImGui::VectorStringGetter, &this->objectPresetName, this->objectPresetName.size());
				ImGui::SameLine();
				if ( ImGui::SmallButton("Add Preset") )
				{
					auto* obj = this->CreateNewObject();
					objectPresetFunc[this->currentSelectedPreset](*obj);
					obj->tag = KG::Utill::HashString(objectPresetName[this->currentSelectedPreset]);
					this->rootNode.GetTransform()->AddChild(obj->GetTransform());
				}
				ImGui::TreePop();
			}
			if ( ImGui::TreeNodeEx("Model Object", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow) )
			{
				materialMatchCache.OnDrawGUI();
				ImGui::SetNextItemWidth(160);
				ImGui::InputHashString("modelID", &modelHash);
				ImGui::SameLine();
				if ( ImGui::SmallButton("Load") )
				{
					this->modelCreator(modelHash, *this, materialMatchCache);
					modelHash.srcString = "";
					modelHash.value = 0;
					materialMatchCache.Clear();
				}
				ImGui::TreePop();
			}
		}
		if ( ImGui::CollapsingHeader("Hierarchy", treeNodeFlag) )
		{
			int count = 0;
			DrawObjectTree(&this->rootNode, currentFocusedObject, count);
		}
	}
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(inspectorSize, viewportSize.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - inspectorSize, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if ( ImGui::Begin("Inspector", &this->isShowGameObjectEdit, flag) )
	{
		if ( ImGui::BeginMenuBar() )
		{
			if ( ImGui::BeginMenu("File") )
			{
				if ( ImGui::MenuItem("Save") && currentFocusedObject )
					ImGuiFileDialog::Instance()->OpenDialog("ChooseObjectSaveKey", " Choose a File", ".xml",
						ImGui::GetCurrentShortPath("Resource\\Objects\\"), currentFocusedObject->tag.srcString, 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::PushItemWidth(160);
		if ( currentFocusedObject )
		{
			currentFocusedObject->DrawGUI(currentGUIContext);
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();


	//


	//FileDialogs

	if ( ImGuiFileDialog::Instance()->Display("ChooseObjectSaveKey") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			currentFocusedObject->SaveToFile(filePathName);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if ( ImGuiFileDialog::Instance()->Display("ChooseSceneSaveKey") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			this->SaveCurrentScene(filePathName);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}


	if ( ImGuiFileDialog::Instance()->Display("ChooseObjectOpenKey") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			auto* newObj = this->CreateNewObject();
			newObj->LoadToFile(filePathName);
			this->rootNode.GetTransform()->AddChild(newObj->GetTransform());
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if ( ImGuiFileDialog::Instance()->Display("ChooseSceneOpenKey") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			this->LoadScene(filePathName);
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}
	return false;
}
