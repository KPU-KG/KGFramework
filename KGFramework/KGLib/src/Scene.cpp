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
	this->objectPool[index].first = true;
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

KG::Core::GameObject* KG::Core::Scene::CreateNewBackObject()
{
	UINT32 id = GetEmptyBackID();
	return this->CreateNewObject(id);
}

UINT32 KG::Core::Scene::GetEmptyBackID()
{
	for ( size_t i = 0; i < this->backActivePool.size(); i++ )
	{
		if ( this->backActivePool[i] == NULL_OBJECT )
		{
			return FlipID(i);
		}
	}
	return FlipID(this->backActivePool.size());
}

size_t KG::Core::Scene::GetBackObjectCount() const
{
	return this->backActivePool.size() - std::count(this->backActivePool.cbegin(), this->backActivePool.cend(), NULL_OBJECT);
}

UINT32 KG::Core::Scene::FlipID(UINT32 frontID)
{
	return NULL_OBJECT - frontID;
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
	UINT backId = FlipID(instanceID);
	if ( this->backActivePool.size() < backId + 1 )
	{
		this->backActivePool.resize(backId + 1, NULL_OBJECT);
	}
	this->backActivePool[backId] = index;

	GameObject* obj = &this->objectPool[index].second;
	obj->SetOwnerScene(this);
	obj->SetInstanceID(instanceID);
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
	UINT32 backID = FlipID(frontID);
	KG::Core::GameObject* result = this->GetFrontObject(frontID);
	if ( result == nullptr )
		result = this->GetBackObject(backID);
	return result;
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
			auto* obj = this->CreateNewObject(id);
			obj->OnDataLoad(objectElement);
			this->objectTree.push_back(obj);
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
	for ( UINT32 i : this->backActivePool )
	{
		if ( i != NULL_OBJECT )
		{
			this->GetBackObject(i)->OnDataSave(sceneElement);
			count++;
		}
	}
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

static void DrawObjectTree(KG::Core::GameObject* node, KG::Core::GameObject*& focused)
{
	bool selected = focused == node;
	if ( !node )
	{
		return;
	}
	if ( ImGui::TreeNodeEx(node->tag.srcString.c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | (!selected ? 0 : ImGuiTreeNodeFlags_Selected)) )
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImU32 col = ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
		if ( ImGui::IsItemClicked() )
		{
			focused = node;
		}
		DrawObjectTree(node->GetChild(), focused);
		ImGui::TreePop();
	}
	else if ( ImGui::IsItemClicked() )
	{
		focused = node;
	}

	DrawObjectTree(node->GetSibling(), focused);
}

bool KG::Core::Scene::OnDrawGUI()
{
	static KG::Core::GameObject* currentFocusedObject = nullptr;
	static constexpr int sceneInfoSize = 250;
	static constexpr int inspectorSize = 400;
	static bool isSelectedSave = false;
	static ImGuiWindowFlags flag = ImGuiWindowFlags_MenuBar;
	static ImGuiTreeNodeFlags treeNodeFlag = ImGuiTreeNodeFlags_DefaultOpen;
	auto viewportSize = ImGui::GetMainViewport()->Size;
	//ImGui::ShowDemoWindow();
	ImGui::SetNextWindowSize(ImVec2(sceneInfoSize, viewportSize.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if ( ImGui::Begin("Scene Info", &this->isShowHierarchy, flag) )
	{
		if ( ImGui::BeginMenuBar() )
		{
			if ( ImGui::BeginMenu("File") )
			{
				if ( ImGui::MenuItem("Clear and New") )
				{

				}
				if ( ImGui::MenuItem("Save") )
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSceneSaveKey", " Choose a File", ".xml"
						, ImGui::GetCurrentShortPath("Resource\\Scenes\\"), "SceneData", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
				if ( ImGui::MenuItem("Load") )
					ImGuiFileDialog::Instance()->OpenDialog("ChooseSceneOpenKey", " Choose a File", ".xml",
						ImGui::GetCurrentShortPath("Resource\\Scenes\\"), "SceneData", 1, nullptr, ImGuiFileDialogFlags_None);
				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu("Object") )
			{
				if ( ImGui::MenuItem("Add New Empty Object To Root") )
				{
					auto* newObj = this->CreateNewBackObject();
					newObj->tag = KG::Utill::HashString("EmptyObject");
					this->objectTree.push_back(newObj);
				}
				if ( ImGui::MenuItem("Add Saved Object To Root") )
				{
					ImGuiFileDialog::Instance()->OpenDialog("ChooseObjectOpenKey", " Choose a File", ".xml",
						ImGui::GetCurrentShortPath("Resource\\Objects\\"), "", 1, nullptr, ImGuiFileDialogFlags_None);
				}
				if ( ImGui::MenuItem("Add Prefab Object To Root") )
				{
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if ( ImGui::CollapsingHeader("Info", treeNodeFlag) )
		{
			ImGui::BulletText("Current Back Object Count : %d", this->GetBackObjectCount());
		}
		if ( ImGui::CollapsingHeader("Scene Object", treeNodeFlag) )
		{
			if ( ImGui::Button("Add Scene Camera Object") )
			{
				auto* obj = this->CreateNewBackObject();
				sceneCameraCreator(*obj);
				this->objectTree.push_back(obj);
			}
			if ( ImGui::Button("Add SkyBox Object") )
			{
				auto* obj = this->CreateNewBackObject();
				skyBoxSetter(this->skyBoxId);
				skyBoxCreator(*obj, this->skyBoxId);
				this->objectTree.push_back(obj);
			}
			ImGui::SameLine();
			this->skyBoxIdProp.OnDrawGUI();

			ImGui::Combo("Preset", &this->currentSelectedPreset, &ImGui::VectorStringGetter, &this->objectPresetName, this->objectPresetName.size());
			if ( ImGui::Button("Add Preset") )
			{
				auto* obj = this->CreateNewBackObject();
				objectPresetFunc[this->currentSelectedPreset](*obj);
				obj->tag = KG::Utill::HashString(objectPresetName[this->currentSelectedPreset]);
				this->objectTree.push_back(obj);
			}
		}
		if ( ImGui::CollapsingHeader("Hierarchy", treeNodeFlag) )
		{
			for ( auto& i : this->objectTree )
			{
				DrawObjectTree(i, currentFocusedObject);
			}
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
					ImGuiFileDialog::Instance()->OpenDialog("ChooseObjectSaveKey"," Choose a File", ".xml",
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
			auto* newObj = this->CreateNewBackObject();
			newObj->LoadToFile(filePathName);
			this->objectTree.push_back(newObj);
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
