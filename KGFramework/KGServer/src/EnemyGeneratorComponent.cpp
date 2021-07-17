#include "pch.h"
// include "EnemyGeneratorComponent.h"
// include "imgui/imgui.h"
// include "MathHelper.h"
// include "GameObject.h"
// include "Scene.h"
// include "Transform.h"
// include "KGServer.h"
// include <random>
// 
// td::random_device rdRegion;
// td::mt19937 genRegion(rdRegion());
// td::uniform_int_distribution<int> randomRegion(0, MAX_REGION - 1);
// 
// 
// oid KG::Component::EnemyGeneratorComponent::OnCreate(KG::Core::GameObject* gameObject)
// 
// 	SBaseComponent::OnCreate(gameObject);
// 
// 
// G::Component::EnemyGeneratorComponent::EnemyGeneratorComponent()
// 
// 
// 
// oid KG::Component::EnemyGeneratorComponent::Update(float timeElapsed) 
// 
// 	bool allDead = true;
// 	for (auto& enemy : enemies) {
// 		if (!enemy->IsDead()) {
// 			allDead = false;
// 			break;
// 		}
// 	}
// 	if (allDead) {
// 		GenerateEnemies(randomRegion(genRegion));
// 		// generate enemies at new region
// 	}
// 
// 
// ool KG::Component::EnemyGeneratorComponent::OnDrawGUI()
// 
// 	if (ImGui::ComponentHeader<KG::Component::SEnemyUnitComponent>()) {
// 		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
// 
// 		ImGuiIO& io = ImGui::GetIO();
// 		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
// 
// 		auto view = this->gameObject->GetScene()->GetMainCameraView();
// 		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
// 
// 		view = Math::Matrix4x4::Transpose(view);
// 		proj = Math::Matrix4x4::Transpose(proj);
// 
// 
// 		// if (this->isUsing()) {
// 		for (int i = 0; i < MAX_REGION; ++i) {
// 			std::string s("REGION" + std::to_string(i));
// 			ImGui::TextDisabled(s.c_str());
// 			std::string ps(s + " Center");
// 			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(ps, this->region[i].position);
// 			std::string rs(s + " Range");
// 			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->region[i].range);
// 
// 			DirectX::XMFLOAT4X4 mat;
// 			DirectX::XMStoreFloat4x4(&mat,
// 				DirectX::XMMatrixScaling(this->region[i].range * 2, 0.1, this->region[i].range * 2) *
// 				DirectX::XMMatrixTranslation(this->region[i].position.x, this->region[i].position.y, this->region[i].position.z)
// 			);
// 
// 			ImGuizmo::DrawCubes(
// 				reinterpret_cast<const float*>(view.m),
// 				reinterpret_cast<const float*>(proj.m),
// 				reinterpret_cast<const float*>(mat.m),
// 				1);
// 		}
// 		// }
// 	}
// 	return false;
// 
// 
// oid KG::Component::EnemyGeneratorComponent::GenerateEnemies(int region)
// 
// 	std::uniform_real_distribution<float> randomPos(-this->region[region].range, this->region[region].range);
// 	auto presetName = "EnemyMech";
// 	auto presetId = KG::Utill::HashString(presetName);
// 
// 	auto* scene = this->gameObject->GetScene();
// 	auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));
// 
// 	DirectX::XMFLOAT3 genPos{
// 		randomPos(genRegion) + this->region[region].position.x,
// 		this->region[region].position.y,
// 		randomPos(genRegion) + this->region[region].position.z
// 	};
// 
// 	KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
// 	auto tag = KG::Utill::HashString(presetName);
// 	addObjectPacket.objectTag = tag;
// 	addObjectPacket.parentTag = 0;
// 	addObjectPacket.presetId = tag;
// 	addObjectPacket.position = genPos;
// 
// 	auto id = this->server->GetNewObjectId();
// 	addObjectPacket.newObjectId = id;
// 	comp->SetNetObjectId(id);
// 	this->server->SetServerObject(id, comp);
// 
// 	auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyUnitComponent>();
// 	enemyCtrl->SetCenter(this->region[region].position);
// 	enemyCtrl->SetWanderRange(this->region[region].range);
// 
// 	enemies.emplace_back(enemyCtrl);
// 
// 	this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
// 	this->BroadcastPacket(&addObjectPacket);
// 