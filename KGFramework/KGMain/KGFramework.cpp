#include <string>
#include <mutex>
#include "ImguiHelper.h"
#include "KGFramework.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "Systems.h"
#include "Debug.h"
#include "GameObject.h"
#include "MaterialMatch.h"
#include "LambdaComponent.h"
#include "SceneCameraComponent.h"
#include "InputManager.h"


KG::GameFramework::GameFramework()
{
	this->system = std::make_unique<Systems>();
	this->scene = std::make_unique<KG::Core::Scene>();
}
KG::GameFramework::~GameFramework()
{
	system.release();
	renderer.release();
	physics.release();
	input.release();
	networkClient.release();
	networkServer.release();
	scene.release();
}
KG::GameFramework::GameFramework(const GameFramework& rhs)
{
	assert(false);
}
KG::GameFramework& KG::GameFramework::operator=(const GameFramework& rhs)
{
	assert(false);
	return *this;
}
KG::GameFramework::GameFramework(GameFramework&& rhs)
{
	assert(false);
}
KG::GameFramework& KG::GameFramework::operator=(GameFramework&& rhs)
{
	assert(false);
	return *this;
}

bool KG::GameFramework::Initialize(const EngineDesc& engineDesc, const Setting& setting)
{
	DebugNormalMessage("Initialize");
	//Desc류 세팅
	this->engineDesc = engineDesc;
	this->setting = setting;

	//Renderer
	this->renderer = std::unique_ptr<KG::Renderer::IKGRenderer>(KG::Renderer::GetD3D12Renderer());

	KG::Renderer::RendererDesc renderDesc;
	renderDesc.hInst = this->engineDesc.hInst;
	renderDesc.hWnd = this->engineDesc.hWnd;

	KG::Renderer::RendererSetting renderSetting;
	renderSetting.clientWidth = this->setting.clientWidth;
	renderSetting.clientHeight = this->setting.clientHeight;
	DebugNormalMessage("RECT : " << renderSetting.clientWidth << " , " << renderSetting.clientHeight);
	renderSetting.isVsync = this->setting.isVsync;

	// Physics
	this->physics = std::unique_ptr<KG::Physics::IPhysicsScene>(KG::Physics::GetPhysicsScene());
	KG::Physics::PhysicsDesc physicsDesc;
#ifdef _DEBUG
	physicsDesc.connectPVD = true;
#elif
	physicsDesc.connectPVD = false;
#endif
	physicsDesc.gravity = 9.81f;

	this->renderer->Initialize(renderDesc, renderSetting);
	this->renderer->PostComponentProvider(this->componentProvider);
	this->physics->Initialize(physicsDesc);
	this->physics->AddFloor(-10);
	this->physics->PostComponentProvider(this->componentProvider);
	this->system->PostComponentProvider(this->componentProvider);
	this->scene->SetComponentProvider(&this->componentProvider);

	this->PostSceneFunction();
	this->scene->InitializeRoot();
	//인풋
	this->input = std::unique_ptr<KG::Input::InputManager>(KG::Input::InputManager::GetInputManager());

	//자원 미리 할당
	this->windowText.reserve(100);

	return true;
}

void KG::GameFramework::PostSceneFunction()
{
	this->scene->AddSkySetter(
		[this](const KG::Utill::HashString& skyBox)
		{
			this->renderer->SetSkymapTextureId(skyBox);
		}
	);
	this->scene->AddSceneCameraObjectCreator(
		[this](KG::Core::GameObject& obj)
		{
			auto* tran = this->system->transformSystem.GetNewComponent();
			auto* cam = this->renderer->GetNewCameraComponent();
			KG::Renderer::RenderTextureDesc renderTextureDesc;
			renderTextureDesc.useDeferredRender = true;
			renderTextureDesc.useCubeRender = false;
			renderTextureDesc.useDepthStencilBuffer = true;
			renderTextureDesc.useRenderTarget = true;
			renderTextureDesc.width = this->setting.clientWidth;
			renderTextureDesc.height = this->setting.clientHeight;
			cam->renderTextureDesc = renderTextureDesc;
			auto* sc = this->system->sceneCameraSystem.GetNewComponent();
			obj.tag = KG::Utill::HashString("SceneCameraObject");
			obj.AddComponent(tran);
			obj.AddComponent(cam);
			obj.AddComponent(sc);
		}
	);
	this->scene->AddSkyBoxObjectCreator(
		[this](KG::Core::GameObject& obj, const KG::Utill::HashString& skyBox)
		{
			auto* tran = this->system->transformSystem.GetNewComponent();
			auto* mat = this->renderer->GetNewMaterialComponent();
			auto* geo = this->renderer->GetNewGeomteryComponent();
			auto* ren = this->renderer->GetNewRenderComponent();
			obj.tag = KG::Utill::HashString("SkyBoxObject");
			geo->AddGeometry(KG::Utill::HashString("cube"));
			mat->PostMaterial(skyBox);
			obj.AddComponent(tran);
			obj.AddComponent(mat);
			obj.AddComponent(geo);
			obj.AddComponent(ren);
		}
	);

	this->scene->AddObjectPreset("EmptyObject",
		[this](KG::Core::GameObject& obj)
		{
			auto* t = this->system->transformSystem.GetNewComponent();
			obj.AddComponent(t);
		}
	);

	this->scene->AddObjectPreset("TileCube",
		[this](KG::Core::GameObject& obj)
		{
			auto* t = this->system->transformSystem.GetNewComponent();
			auto* g = this->renderer->GetNewGeomteryComponent();
			g->AddGeometry(KG::Utill::HashString("cube"));
			auto* m = this->renderer->GetNewMaterialComponent();
			m->PostMaterial(KG::Utill::HashString("PBRTile"));
			auto* r = this->renderer->GetNewRenderComponent();
			obj.AddComponent(t);
			obj.AddTemporalComponent(g);
			obj.AddTemporalComponent(m);
			obj.AddTemporalComponent(r);
		}
	);

	this->scene->AddModelPreset("Vector",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("arms");
			KG::Resource::MaterialSet s;
			s.emplace_back("vectorNormal"_id);
			a.AddMaterial("Mesh_Vector"_id, s);
			s.clear();
			s.emplace_back("vectorSight"_id);
			a.AddMaterial("Cube.006"_id, s);

			return std::make_pair(
				KG::Utill::HashString("Vector.FBX"),
				std::move(a)
			);
		},
		[this](KG::Core::GameObject& obj)
		{
			auto* boneVector = obj.FindChildObject("Bone_Vector"_id);
			auto* sight = obj.FindChildObject("Cube.006"_id);
			sight->GetTransform()->ExtractThisNode();
			boneVector->GetTransform()->AddChild(sight->GetTransform());
			sight->GetTransform()->SetPosition(0.042f, -6.841, 4.2f);
			sight->GetTransform()->SetEulerDegree(-90,-90,0);
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation(KG::Utill::HashString("Vector@Idle.FBX"_id));
			ctrl->RegisterAnimation(KG::Utill::HashString("Vector@Reload.FBX"_id));
			ctrl->RegisterAnimation(KG::Utill::HashString("Vector@ReloadEmpty.FBX"_id));
			ctrl->RegisterAnimation(KG::Utill::HashString("Vector@Fire.FBX"_id));
			ctrl->RegisterAnimation(KG::Utill::HashString("Vector@Fire Aim.FBX"_id));

			ctrl->SetAnimation(KG::Utill::HashString("Vector@Idle.FBX"_id),-1.0f, 1.0f);
			ctrl->SetDefaultAnimation(KG::Utill::HashString("Vector@Idle.FBX"_id));
			ctrl->SetIgnoreTranslate(false);
			ctrl->SetIgnoreScale(true);
			obj.AddComponent(ctrl);
			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
		}
		);
	this->scene->AddModelPreset("CS5",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("arms");

			return std::make_pair(
				KG::Utill::HashString("CS5.FBX"),
				std::move(a)
			);
		},
		[this](KG::Core::GameObject& obj)
		{
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation(KG::Utill::HashString("CS5@Idle.FBX"_id));

			ctrl->SetAnimation(KG::Utill::HashString("CS5@Idle.FBX"_id));
			ctrl->SetDefaultAnimation(KG::Utill::HashString("CS5@Idle.FBX"_id));
			ctrl->SetIgnoreScale(true);
			obj.AddComponent(ctrl);
			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
		}
		);

	this->scene->AddModelPreset("ACR",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("arms");

			return std::make_pair(
				KG::Utill::HashString("ACR.FBX"),
				std::move(a)
			);
		},
		[this](KG::Core::GameObject& obj)
		{
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation(KG::Utill::HashString("ACR@Idle.FBX"_id));

			ctrl->SetAnimation(KG::Utill::HashString("ACR@Idle.FBX"_id));
			ctrl->SetDefaultAnimation(KG::Utill::HashString("ACR@Idle.FBX"_id));
			ctrl->SetIgnoreScale(true);
			obj.AddComponent(ctrl);
			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
		}
		);

	this->scene->AddModelPreset("PlayerArms",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("arms");

			return std::make_pair(
				KG::Utill::HashString("Arms.FBX"),
				std::move(a)
			);
		},
		[this](KG::Core::GameObject& obj)
		{
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation(KG::Utill::HashString("Arms@Idle.FBX"_id));

			ctrl->SetAnimation(KG::Utill::HashString("Arms@Idle.FBX"_id));
			ctrl->SetDefaultAnimation(KG::Utill::HashString("Arms@Idle.FBX"_id));
			obj.AddComponent(ctrl);
			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
		}
		);


	this->scene->AddModelPreset("PlayerCharacter",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("soldierHead");
			a.defaultMaterial.emplace_back("soldierBody");

			return std::make_pair(
				KG::Utill::HashString("soldier"),
				std::move(a)
			);
		}
		,
			[this](KG::Core::GameObject& obj)
		{
			obj.GetTransform()->SetPosition(10, 2, 10);
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation("Soldier@Standing.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkRight.fbx"_id);

			ctrl->SetAnimation(KG::Utill::HashString("Soldier@Standing.fbx"_id));
			ctrl->SetDefaultAnimation(KG::Utill::HashString("Soldier@Standing.fbx"_id));
			ctrl->SetIgnoreTranslate(true);
			ctrl->SetIgnoreScale(true);
			obj.AddComponent(ctrl);

			auto* cameraObj = this->scene->CreateNewTransformObject();
			cameraObj->tag = KG::Utill::HashString("FPCamera");

			auto* cam = this->renderer->GetNewCameraComponent();
			KG::Renderer::RenderTextureDesc renderTextureDesc;
			renderTextureDesc.useDeferredRender = true;
			renderTextureDesc.useCubeRender = false;
			renderTextureDesc.useDepthStencilBuffer = true;
			renderTextureDesc.useRenderTarget = true;
			renderTextureDesc.width = this->setting.clientWidth;
			renderTextureDesc.height = this->setting.clientHeight;
			cam->renderTextureDesc = renderTextureDesc;
			cam->SetFovY(90.0f);

			cameraObj->AddComponent(cam);
			cameraObj->GetTransform()->SetPosition(0.230, 1.45, 0.496);

			auto* arms = this->scene->CallPreset("Vector");
			arms->GetTransform()->SetEulerRadian(0, 0, 0);
			arms->GetTransform()->SetPosition(0, 0, -0.1);
			arms->GetTransform()->SetScale(1.2, 1, 1);
			arms->GetTransform()->GetChild()->SetPosition(0.0, 0.0, 0.0f);
			arms->GetTransform()->GetChild()->SetEulerDegree(0, 90.0f, 0);

			cameraObj->GetTransform()->AddChild(arms->GetTransform());

			obj.GetTransform()->AddChild(cameraObj->GetTransform());

			auto* player = this->system->playerComponentSystem.GetNewComponent();
			obj.AddComponent(player);

			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
			obj.GetTransform()->SetPosition(10.0, 0.00f, 5.00f);

			auto* phy = this->physics->GetNewDynamicRigidComponent();
			phy->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
				DebugNormalMessage("Collide");
				});
			phy->SetUpdateCallback([this, obj, phy]() {
				if (this->input->IsTouching(VK_LBUTTON) && this->input->GetMouseCapture())
				{
					auto* tran = obj.GetTransform();
					DirectX::XMFLOAT3 start = tran->GetPosition();
					// start = Math::Vector3::Add(start, tran->GetLook() * 2);
					auto* other = this->physics->QueryRaycast(start, tran->GetLook(), 100, phy->GetId());
					if (other != nullptr) {
						if (other->IsDynamic()) {
							other->AddForce(tran->GetLook(), 15);
						}
					}
				}
				});
			obj.AddTemporalComponent(phy);

		}
		);

	this->scene->AddModelPreset("TeamCharacter",
		[]()
		{
			KG::Resource::MaterialMatch a;
			a.defaultMaterial.emplace_back("soldierHead");
			a.defaultMaterial.emplace_back("soldierBody");

			return std::make_pair(
				KG::Utill::HashString("soldier"),
				std::move(a)
			);
		}
		,
			[this](KG::Core::GameObject& obj)
		{
			auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

			ctrl->RegisterAnimation("Soldier@Standing.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@SprintForwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkBackwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForward.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForwardLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkForwardRight.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkLeft.fbx"_id);
			ctrl->RegisterAnimation("Soldier@WalkRight.fbx"_id);

			ctrl->SetAnimation(KG::Utill::HashString("Soldier@Standing.fbx"_id));
			ctrl->SetDefaultAnimation(KG::Utill::HashString("Soldier@Standing.fbx"_id));
			ctrl->SetIgnoreScale(false);
			obj.AddComponent(ctrl);

			obj.GetTransform()->GetChild()->SetScale(0.01f, 0.01f, 0.01f);
		}
		);

	this->scene->AddObjectPreset("Directional Light",
		[this](KG::Core::GameObject& obj)
		{
			auto* t = this->system->transformSystem.GetNewComponent();
			auto* l = this->renderer->GetNewLightComponent();
			l->SetDirectionalLight(DirectX::XMFLOAT3(1.2f, 1.2f, 1.2f), DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f));
			obj.AddComponent(t);
			obj.AddComponent(l);
		}
	);
	this->scene->AddObjectPreset("Ambient Processor",
		[this](KG::Core::GameObject& obj)
		{
			auto* t = this->system->transformSystem.GetNewComponent();
			auto* m = this->renderer->GetNewMaterialComponent();
			auto* g = this->renderer->GetNewGeomteryComponent();
			auto* r = this->renderer->GetNewRenderComponent();
			m->PostShader(KG::Utill::HashString("AmbientLight"));
			g->AddGeometry(KG::Utill::HashString("lightPlane"_id));
			obj.AddComponent(t);
			obj.AddComponent(m);
			obj.AddComponent(g);
			obj.AddComponent(r);
		}
	);
	this->scene->AddModelCreator(
		[this](const KG::Utill::HashString& modelID, KG::Core::Scene& scene, const KG::Resource::MaterialMatch& material)
		{
			return this->renderer->LoadFromModel(modelID, scene, material);
		}
	);

	this->scene->AddCameraMatrixGetter(
		[](KG::Component::IComponent* comp)
		{
			auto* camera = static_cast<KG::Component::CameraComponent*>(comp);
			return camera->GetView();
		},
		[](KG::Component::IComponent* comp)
		{
			auto* camera = static_cast<KG::Component::CameraComponent*>(comp);
			return camera->GetProj();
		}
		);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int KG::GameFramework::WinProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool ret = ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
	input->HandlingInputProc(hWnd, message, wParam, lParam);
	return ret;
}

void KG::GameFramework::UIPreRender()
{
	this->renderer->PreRenderUI();
	guiContext = (ImGuiContext*)this->renderer->GetImGUIContext();
	ImGui::SetCurrentContext(guiContext);
	this->input->SetUIContext(guiContext);
	this->physics->SetGUIContext(guiContext);
	// this->physics->
}

void KG::GameFramework::UIRender()
{
	this->scene->DrawGUI(guiContext);

	static constexpr int sceneInfoSize = 250;
	static constexpr int inspectorSize = 400;
	auto viewportSize = ImGui::GetWindowViewport()->Size;
	ImGui::SetNextWindowSize(ImVec2(sceneInfoSize, viewportSize.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if ( ImGui::Begin("GameFramework") )
	{
		if ( ImGui::Button("Reset Scene") )
		{
			this->scene.release();
			this->scene = std::make_unique<KG::Core::Scene>();
		}
	}
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(sceneInfoSize, viewportSize.y), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowBgAlpha(0.8f);
	if ( ImGui::Begin("Network") )
	{
		if ( ImGui::Button("Open Console") )
		{
			if ( !AllocConsole() )
				MessageBox(NULL, L"The console window was not created", NULL, MB_ICONEXCLAMATION);
			FILE* console;
			freopen_s(&console, "CONIN$", "r", stdin);
			freopen_s(&console, "CONOUT$", "w", stderr);
			freopen_s(&console, "CONOUT$", "w", stdout);
			printf("DEBUG CONSOLE OPEN\n");
			std::cout.clear();
		}
		if ( this->networkClient == nullptr && this->networkServer == nullptr )
		{
			if ( ImGui::Button("Start Network Client") )
			{
				this->networkClient = std::unique_ptr<KG::Server::INetwork>(KG::Server::GetNetwork());
				this->networkClient->SetGUIContext(this->guiContext);
				this->networkClient->Initialize();
				this->networkClient->SetScene(this->scene.get());
				this->networkClient->SetInputManager(this->input.get());
				this->networkClient->PostComponentProvider(this->componentProvider);

				this->PostNetworkFunction();

				//Hard Code
				this->scene->AddSceneComponent("CGameManagerComponent"_id, this->componentProvider.GetComponent("CGameManagerComponent"_id));
			}
			if ( ImGui::Button("Start Network Server") )
			{
				this->networkServer = std::unique_ptr<KG::Server::IServer>(KG::Server::GetServer());
				this->networkServer->SetGUIContext(this->guiContext);
				this->networkServer->Initialize();
				this->networkServer->PostComponentProvider(this->componentProvider);

				this->PostServerFunction();

				//Hard Code
				this->scene->AddSceneComponent("SGameManagerComponent"_id, this->componentProvider.GetComponent("SGameManagerComponent"_id));
			}
		}
		else
		{
			if ( this->networkClient ) this->networkClient->DrawImGUI();
			if ( this->networkServer ) this->networkServer->DrawImGUI();
		}

	}
	ImGui::End();
}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();

	this->ServerProcess();

	this->UIPreRender();
	this->UIRender();
	this->input->ProcessInput(this->engineDesc.hWnd);
	this->system->OnUpdate(this->timer.GetTimeElapsed());
	this->ServerUpdate(this->timer.GetTimeElapsed());
	if ( this->scene->isStartGame )
	{
		this->renderer->Update(this->timer.GetTimeElapsed());
	}
	this->physics->Advance(this->timer.GetTimeElapsed());
	this->renderer->Render();

	this->ServerProcessEnd();
	this->input->PostProcessInput();
}

void KG::GameFramework::ServerProcess()
{
	if ( this->networkClient && this->networkClient->IsConnected() )
	{
		this->networkClient->TryRecv();
	}
	else if ( this->networkServer && this->networkServer->isStarted() )
	{
		this->networkServer->LockWorld();
	}
}

void KG::GameFramework::ServerUpdate(float elapsedTime)
{
	if ( this->networkClient && this->networkClient->IsConnected() )
	{
		this->networkClient->Update(elapsedTime);
	}
	if ( this->networkServer && this->networkServer->isStarted() )
	{
		this->networkServer->Update(elapsedTime);
	}
}

void KG::GameFramework::ServerProcessEnd()
{
	if ( this->networkServer && this->networkServer->isStarted() )
	{
		this->networkServer->UnlockWorld();
	}
}

void KG::GameFramework::OnClose()
{
	this->setting.Save(this->setting);
}

void KG::GameFramework::UpdateWindowText()
{
	static float duration = 0.0f;
	duration += this->timer.GetTimeElapsed();
	if ( duration >= 1.0f )
	{
		auto frame = this->timer.GetFrameRate();
		::_itow_s(frame, this->windowText.data() + 21, 10, 10);
		::SetWindowText(this->engineDesc.hWnd, this->windowText.data());
		duration = 0.0f;
	}
}
