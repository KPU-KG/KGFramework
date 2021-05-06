#include "ImguiHelper.h"
#include "Transform.h"
#include "GameObject.h"
#include "Scene.h"

void KG::Core::Scene::DrawObjectTree(KG::Core::GameObject* node, KG::Core::GameObject*& focused, int count)
{
	bool selected = focused == node;
	if ( !node )
	{
		return;
	}
	count += 1;
	ImGui::PushID(count);
	bool opend = ImGui::TreeNodeEx(node->tag.srcString.c_str(),
		ImGuiTreeNodeFlags_OpenOnArrow | (count == 1 ? ImGuiTreeNodeFlags_DefaultOpen : 0) | (!selected ? 0 : ImGuiTreeNodeFlags_Selected)
	);
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
	ImGui::PopID();
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

}

static inline bool exists(const std::string& name)
{
	FILE* file = nullptr;
	fopen_s(&file, name.c_str(), "r");
	if ( file )
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
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
	//ImGui::ShowDemoWindow();
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

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
				if ( ImGui::MenuItem("Exit") )
					exit(0);
				ImGui::EndMenu();
			}
			if ( ImGui::BeginMenu("Resource") )
			{
				if ( ImGui::MenuItem("Make Material From Directory") )
					ImGuiFileDialog::Instance()->OpenDialog("MakeMaterial", " Choose a Directory", nullptr
						, ImGui::GetCurrentShortPath("Resource\\Texture\\"), "", 1, nullptr);
				if ( ImGui::MenuItem("Make Add Particle Texture From Directory") )
					ImGuiFileDialog::Instance()->OpenDialog("MakeParticleTexture", " Choose a File", ".DDS{.dds,.DDS},"
						, ImGui::GetCurrentShortPath("Resource\\Texture\\Particles"), "", 0, nullptr);
				if ( ImGui::MenuItem("Make Geometry From File") )
					ImGuiFileDialog::Instance()->OpenDialog("MakeGeometry", " Choose a File", ".FBX{.fbx,.FBX},"
						, ImGui::GetCurrentShortPath("Resource\\Geometry\\"), "", 1, nullptr);
				if ( ImGui::MenuItem("Make Model From File") )
					ImGuiFileDialog::Instance()->OpenDialog("MakeModel", " Choose a File", ".FBX{.fbx,.FBX}"
						, ImGui::GetCurrentShortPath("Resource\\Geometry\\"), "", 0, nullptr);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if ( ImGui::CollapsingHeader("Info", treeNodeFlag) )
		{
			ImGui::BulletText("Current Object Count : %d", this->GetObjectCount());
			ImGui::Checkbox("Update Loop", &this->isStartGame);
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
					auto* obj = this->CallPreset(KG::Utill::HashString("EmptyObject"));
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
					KG::Core::GameObject* obj = nullptr;
					if ( objectPresetModel[this->currentSelectedPreset] != nullptr )
					{
						auto [modelId, materialMach] = this->objectPresetModel[this->currentSelectedPreset]();
						obj = this->modelCreator(modelId, *this, materialMach);
					}
					else
					{
						obj = this->CreateNewObject();
					}
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
					auto* obj = this->modelCreator(modelHash, *this, materialMatchCache);
					obj->tag = modelHash;
					this->rootNode.GetTransform()->AddChild(obj->GetTransform());
					modelHash.srcString = "";
					modelHash.value = 0;
					materialMatchCache.Clear();
				}
				ImGui::TreePop();
			}
		}
		bool isHierarchyOpen = ImGui::CollapsingHeader("Hierarchy", treeNodeFlag);
		if ( ImGui::BeginDragDropTarget() )
		{
			auto* payLoad = ImGui::AcceptDragDropPayload("_GameObject");
			if ( payLoad )
			{
				KG::Core::GameObject* dragSource = *static_cast<KG::Core::GameObject**>(payLoad->Data);
				if ( dragSource && dragSource->GetTransform() )
				{
					dragSource->GetTransform()->ExtractThisNode();
					dragSource->Destroy();
				}
			}
			ImGui::EndDragDropTarget();
		}
		if ( isHierarchyOpen )
		{
			DrawObjectTree(&this->rootNode, currentFocusedObject);
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
			auto* newObj = this->CreateNewTransformObject();
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

	if ( ImGuiFileDialog::Instance()->Display("MakeMaterial") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGui::ShortPathToLongPath(ImGuiFileDialog::Instance()->GetFilePathName());
			std::string resourcePath = filePathName.substr(filePathName.rfind("Resource"));
			std::string materialName = filePathName.substr(filePathName.rfind("\\") + 1);
			for ( auto& i : resourcePath )
			{
				if ( i == '\\' )
				{
					i = '/';
				}
			}

			bool hasColor = exists(resourcePath + "\\color.dds");
			bool hasNormal = exists(resourcePath + "\\normal.dds");
			bool hasRough = exists(resourcePath + "\\rough.dds");
			bool hasMetalic = exists(resourcePath + "\\metalic.dds");

			//Create Textures
			{
				tinyxml2::XMLDocument textureSet;
				textureSet.LoadFile("Resource/TextureSet.xml");
				textureSet.FirstChildElement("TextureSet")->InsertNewComment(materialName.c_str());
				auto elementCreator = [&](const std::string& name)
				{
					auto* texEle = textureSet.FirstChildElement("TextureSet")->InsertNewChildElement("Texture");
					texEle->SetAttribute("id", (materialName + "_" + name).c_str());
					texEle->SetAttribute("fileDir", (resourcePath + "/" + name + ".dds").c_str());
					texEle->SetAttribute("dimension", "Texture2D");
					texEle->SetAttribute("format", "DDS");
				};

				if ( hasColor )
					elementCreator("color");
				if ( hasNormal )
					elementCreator("normal");
				if ( hasRough )
					elementCreator("rough");
				if ( hasMetalic )
					elementCreator("metalic");

				textureSet.SaveFile("Resource/TextureSet.xml");
			}
			//Create Material
			{
				tinyxml2::XMLDocument materialSet;
				materialSet.LoadFile("Resource/MaterialSet.xml");
				auto* matEle = materialSet.FirstChildElement("MaterialSet")->InsertNewChildElement("Material");
				matEle->SetAttribute("id", materialName.c_str());
				matEle->SetAttribute("shaderID", "PBRDefault");
				{
					auto nodeFunc = [&](bool has, const std::string& type, const std::string& none)
					{
						auto* node = matEle->InsertNewChildElement("Texture");
						node->SetAttribute("byte", 4);
						node->SetAttribute("comment", (type + "Texture").c_str());
						node->SetAttribute("id", (has ? (materialName + "_" + type) : none).c_str());
					};
					nodeFunc(hasColor, "color", "one");
					nodeFunc(hasNormal, "normal", "one");
					nodeFunc(hasMetalic, "metalic", "zero");
					nodeFunc(hasRough, "rough", "zero");
					{
						auto* node = matEle->InsertNewChildElement("FLOAT1");
						node->SetAttribute("byte", 4);
						node->SetAttribute("comment", "Specular");
						node->SetAttribute("x", 0.5f);
					}

					{
						auto* node = matEle->InsertNewChildElement("FLOAT2");
						node->SetAttribute("byte", 8);
						node->SetAttribute("comment", "UVsize");
						node->SetAttribute("x", 1);
						node->SetAttribute("y", 1);
					}
					{
						auto* node = matEle->InsertNewChildElement("Padding");
						node->SetAttribute("byte", 4);
					}
				}
				materialSet.SaveFile("Resource/MaterialSet.xml");
			}
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if ( ImGuiFileDialog::Instance()->Display("MakeGeometry") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{
			std::string filePathName = ImGui::ShortPathToLongPath(ImGuiFileDialog::Instance()->GetFilePathName());
			std::string resourcePath = filePathName.substr(filePathName.rfind("Resource"));
			std::string geometryName = filePathName.substr(filePathName.rfind("\\") + 1, filePathName.length() - 4);
			for ( auto& i : resourcePath )
			{
				if ( i == '\\' )
				{
					i = '/';
				}
			}

			//Create Textures
			{
				tinyxml2::XMLDocument textureSet;
				textureSet.LoadFile("Resource/GeometrySet.xml");
				textureSet.FirstChildElement("GeometrySet")->InsertNewComment(geometryName.c_str());
				auto* texEle = textureSet.FirstChildElement("GeometrySet")->InsertNewChildElement("Geometry");
				texEle->SetAttribute("id", (geometryName).c_str());
				texEle->SetAttribute("fileDir", resourcePath.c_str());
				texEle->SetAttribute("rawMesh", true);
				textureSet.SaveFile("Resource/GeometrySet.xml");
			}
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if ( ImGuiFileDialog::Instance()->Display("MakeModel") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{

			auto map = ImGuiFileDialog::Instance()->GetSelection();
			for ( auto [fileName, getFilePathName] : map )
			{
				std::string filePathName = ImGui::ShortPathToLongPath(getFilePathName);
				std::string resourcePath = filePathName.substr(filePathName.rfind("Resource"));
				std::string geometryName = filePathName.substr(filePathName.rfind("\\") + 1, filePathName.length() - 4);
				for ( auto& i : resourcePath )
				{
					if ( i == '\\' )
					{
						i = '/';
					}
				}

				{
					tinyxml2::XMLDocument textureSet;
					textureSet.LoadFile("Resource/GeometrySet.xml");
					textureSet.FirstChildElement("GeometrySet")->InsertNewComment(geometryName.c_str());
					auto* texEle = textureSet.FirstChildElement("GeometrySet")->InsertNewChildElement("Geometry");
					texEle->SetAttribute("id", (geometryName).c_str());
					texEle->SetAttribute("fileDir", resourcePath.c_str());
					texEle->SetAttribute("rawMesh", false);
					textureSet.SaveFile("Resource/GeometrySet.xml");
				}
			}
		}
		// close
		ImGuiFileDialog::Instance()->Close();
	}

	if ( ImGuiFileDialog::Instance()->Display("MakeParticleTexture") )
	{
		// action if OK
		if ( ImGuiFileDialog::Instance()->IsOk() )
		{

			auto map = ImGuiFileDialog::Instance()->GetSelection();
			for ( auto [fileName, getFilePathName] : map )
			{
				std::string filePathName = ImGui::ShortPathToLongPath(getFilePathName);
				std::string ddsFilePath = filePathName.substr(filePathName.rfind("Resource"));
				for ( auto& i : ddsFilePath )
				{
					if ( i == '\\' )
					{
						i = '/';
					}
				}

				//Create Textures
				{
					tinyxml2::XMLDocument textureSet;
					textureSet.LoadFile("Resource/TextureSet.xml");
					textureSet.FirstChildElement("TextureSet")->InsertNewComment(ddsFilePath.c_str());

					auto* texEle = textureSet.FirstChildElement("TextureSet")->InsertNewChildElement("Texture");
					texEle->SetAttribute("id", fileName.c_str());
					texEle->SetAttribute("fileDir", ddsFilePath.c_str());
					texEle->SetAttribute("dimension", "Texture2D");
					texEle->SetAttribute("format", "DDS");

					textureSet.SaveFile("Resource/TextureSet.xml");
				}
			}
		}
		// close
		ImGuiFileDialog::Instance()->Close();
	}

	return false;
}
