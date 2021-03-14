#include <string>
#include "ImguiHelper.h"
#include "KGFramework.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "Systems.h"
#include "Debug.h"
#include "GameObject.h"
#include "LambdaComponent.h"
#include "InputManager.h"

KG::GameFramework::GameFramework()
{
	this->system = std::make_unique<Systems>();
}
KG::GameFramework::~GameFramework()
{
}
KG::GameFramework::GameFramework( const GameFramework& rhs )
{
	assert( false );
}
KG::GameFramework& KG::GameFramework::operator=( const GameFramework& rhs )
{
	assert( false );
	return *this;
}
KG::GameFramework::GameFramework( GameFramework&& rhs )
{
	assert( false );
}
KG::GameFramework& KG::GameFramework::operator=( GameFramework&& rhs )
{
	assert( false );
	return *this;
}

bool KG::GameFramework::Initialize( const EngineDesc& engineDesc, const Setting& setting )
{
	DebugNormalMessage( "Initialize" );
	//Desc류 세팅
	this->engineDesc = engineDesc;
	this->setting = setting;

	//Renderer
	this->renderer = std::unique_ptr<KG::Renderer::IKGRenderer>( KG::Renderer::GetD3D12Renderer() );

	KG::Renderer::RendererDesc renderDesc;
	renderDesc.hInst = this->engineDesc.hInst;
	renderDesc.hWnd = this->engineDesc.hWnd;

	KG::Renderer::RendererSetting renderSetting;
	renderSetting.clientWidth = this->setting.clientWidth;
	renderSetting.clientHeight = this->setting.clientHeight;
	DebugNormalMessage( "RECT : " << renderSetting.clientWidth << " , " << renderSetting.clientHeight );
	renderSetting.isVsync = this->setting.isVsync;

	this->renderer->Initialize( renderDesc, renderSetting );
	this->renderer->PostComponentProvider(this->componentProvider);
	this->system->PostComponentProvider(this->componentProvider);
	this->scene.SetComponentProvider(&this->componentProvider);

	this->scene.AddSceneCameraObjectCreator(
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
			auto* lam = this->system->lambdaSystem.GetNewComponent();
			static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
				[](KG::Core::GameObject* gameObject, float elapsedTime)
				{
					auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
					using namespace KG::Input;
					auto input = InputManager::GetInputManager();
					float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
					if ( ImGui::IsAnyItemFocused() )
					{
						return;
					}
					if ( input->IsTouching('W') )
					{
						trans->Translate(trans->GetLook() * speed * elapsedTime);
					}
					if ( input->IsTouching('A') )
					{
						trans->Translate(trans->GetRight() * speed * elapsedTime * -1);
					}
					if ( input->IsTouching('S') )
					{
						trans->Translate(trans->GetLook() * speed * elapsedTime * -1);
					}
					if ( input->IsTouching('D') )
					{
						trans->Translate(trans->GetRight() * speed * elapsedTime);
					}
					if ( input->IsTouching('E') )
					{
						trans->Translate(trans->GetUp() * speed * elapsedTime);
					}
					if ( input->IsTouching('Q') )
					{
						trans->Translate(trans->GetUp() * speed * elapsedTime * -1);
					}

					if ( input->IsTouching(VK_RBUTTON) )
					{
						auto delta = input->GetDeltaMousePosition();
						if ( delta.x )
						{
							trans->RotateAxis(Math::up, delta.x * 0.3f);
						}
						if ( delta.y )
						{
							trans->RotateAxis(trans->GetRight(), delta.y * 0.3f);
						}
					}
					auto worldPos = trans->GetWorldPosition();
					DebugNormalMessage("LambdaTransform : " << worldPos);
				}
			);
			obj.tag = KG::Utill::HashString("SceneCameraObject");
			obj.AddComponent(tran);
			obj.AddComponent(cam);
			obj.AddComponent(lam);
		}
	);
	this->scene.AddSkyBoxObjectCreator(
		[this](KG::Core::GameObject& obj, const KG::Utill::HashString& skyBox)
		{
			auto* tran = this->system->transformSystem.GetNewComponent();
			auto* mat = this->renderer->GetNewMaterialComponent();
			auto* geo = this->renderer->GetNewGeomteryComponent();
			auto* ren = this->renderer->GetNewRenderComponent();
			obj.tag = KG::Utill::HashString("SkyBoxObject");
			geo->geometryID = KG::Utill::HashString("cube");
			mat->materialID = skyBox;
			obj.AddComponent(tran);
			obj.AddComponent(mat);
			obj.AddComponent(geo);
			obj.AddComponent(ren);
			this->renderer->SetSkymapTextureId(KG::Utill::HashString("skySnow"));
		}
	);


	//인풋
	this->input = std::unique_ptr<KG::Input::InputManager>( KG::Input::InputManager::GetInputManager() );

	//자원 미리 할당
	this->windowText.reserve( 100 );

	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int KG::GameFramework::WinProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
}

void KG::GameFramework::UIRender()
{
	auto* currentContext = (ImGuiContext*)this->renderer->GetImGUIContext();
	ImGui::SetCurrentContext(currentContext);
	this->scene.DrawGUI(currentContext);
}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();
	DebugNormalMessage( "OnUpdatedProcess");
	this->input->ProcessInput( this->engineDesc.hWnd );
	this->renderer->PreRenderUI();
	this->UIRender();
	this->system->OnUpdate( this->timer.GetTimeElapsed() );
	if ( this->scene.isStartGame )
	{
		this->renderer->Update( this->timer.GetTimeElapsed() );
	}
	this->renderer->Render();
}

void KG::GameFramework::OnClose()
{
	this->setting.Save( this->setting );
}

void KG::GameFramework::UpdateWindowText()
{
	static float duration = 0.0f;
	duration += this->timer.GetTimeElapsed();
	if ( duration >= 1.0f )
	{
		auto frame = this->timer.GetFrameRate();
		::_itow_s( frame, this->windowText.data() + 21, 10, 10 );
		::SetWindowText( this->engineDesc.hWnd, this->windowText.data() );
		duration = 0.0f;
	}
}
