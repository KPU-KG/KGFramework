#include "KGFramework.h"
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
	renderSetting.isVsync = this->setting.isVsync;

	this->renderer->Initialize( renderDesc, renderSetting );

	//인풋
	this->input = std::unique_ptr<KG::Input::InputManager>( KG::Input::InputManager::GetInputManager() );

	//자원 미리 할당
	this->windowText.reserve( 100 );

	OnTestInit();

	return true;
}

void KG::GameFramework::OnTestInit()
{
	static KG::Core::GameObject testGameObject;
	static KG::Core::GameObject testPlaneObject;
	static KG::Core::GameObject testCubeObjects[10000];
	static KG::Core::GameObject testCameraObject;
	static KG::Core::GameObject testLightObject;
	static KG::Core::GameObject testPointLightObjects[10000];


	constexpr auto texOne = "PBRMetal"_id;
	constexpr auto texTwo = "TileMaterial"_id;
	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[]( KG::Core::GameObject* gameObject, float elapsedTime )
			{
				static bool tos = false;
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				using namespace KG::Input;

				if ( InputManager::GetInputManager()->GetKeyState( VK_SPACE ) == KeyState::Down )
				{
					DebugNormalMessage( "Toggle Changed" );
					tos = !tos;
				}

				if ( tos )
					trans->RotateEuler( 0.0f, 180.0f * elapsedTime, 0.0f );

				if ( InputManager::GetInputManager()->GetKeyState( VK_ESCAPE ) == KeyState::Down )
				{
					PostQuitMessage( 0 );
				}
			}
		);
		auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( texOne ) );
		auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( "test"_id ) );
		auto* ren = this->renderer->GetNewRenderComponent();
		testGameObject.name = "meshObject0";
		testGameObject.AddComponent( tran );
		testGameObject.AddComponent( mat );
		testGameObject.AddComponent( lam );
		testGameObject.AddComponent( geo );
		testGameObject.AddComponent( ren );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->Translate( 0, 0, 0.0f );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->RotateEuler( 0.0f, 0.0f, 0.0f );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->SetScale( 0.05f, 0.05f, 0.05f );

	}
	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( texOne ) );
		auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( "plane"_id ) );
		auto* ren = this->renderer->GetNewRenderComponent();
		testPlaneObject.name = "plane";
		testPlaneObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		testPlaneObject.AddComponent( mat );
		testPlaneObject.AddComponent( geo );
		testPlaneObject.AddComponent( ren );
		testPlaneObject.GetComponent<KG::Component::TransformComponent>()->Translate( 15.0f, -0.5f, 15.0f );
		testPlaneObject.GetComponent<KG::Component::TransformComponent>()->RotateEuler( 90.0f, 0.0f, 0.0f );
		testPlaneObject.GetComponent<KG::Component::TransformComponent>()->SetScale( DirectX::XMFLOAT3( 1, 1, 1 ) * 20 );

	}
	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* cam = this->renderer->GetNewCameraComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[]( KG::Core::GameObject* gameObject, float elapsedTime )
			{
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				using namespace KG::Input;
				auto input = InputManager::GetInputManager();
				float speed = input->IsTouching( VK_LSHIFT ) ? 6.0f : 2.0f;
				if ( input->IsTouching( 'W' ) )
				{
					trans->Translate( trans->GetLook() * speed * elapsedTime );
				}
				if ( input->IsTouching( 'A' ) )
				{
					trans->Translate( trans->GetRight() * speed * elapsedTime * -1 );
				}
				if ( input->IsTouching( 'S' ) )
				{
					trans->Translate( trans->GetLook() * speed * elapsedTime * -1 );
				}
				if ( input->IsTouching( 'D' ) )
				{
					trans->Translate( trans->GetRight() * speed * elapsedTime );
				}
				if ( input->IsTouching( 'E' ) )
				{
					trans->Translate( trans->GetUp() * speed * elapsedTime );
				}
				if ( input->IsTouching( 'Q' ) )
				{
					trans->Translate( trans->GetUp() * speed * elapsedTime * -1 );
				}

				if ( input->IsTouching( VK_RBUTTON ) )
				{
					auto delta = input->GetDeltaMousePosition();
					if ( delta.x )
					{
						trans->RotateAxis( Math::up, delta.x * 0.3f );
					}
					if ( delta.y )
					{
						trans->RotateAxis( trans->GetRight(), delta.y * 0.3f );
					}
				}
			}
		);
		testCameraObject.name = "camera";
		testCameraObject.AddComponent( tran );
		testCameraObject.AddComponent( cam );
		testCameraObject.AddComponent( lam );
		testCameraObject.GetComponent<KG::Component::TransformComponent>()->Translate( 0, 0, -2.0f );
	}

	constexpr size_t cdas = 30;
	for ( size_t y = 0; y < cdas; y++ )
	{
		for ( size_t x = 0; x < cdas; x++ )
		{
			size_t index = cdas * y + x;
			{
				auto* tran = this->system->transformSystem.GetNewComponent();
				auto* lam = this->system->lambdaSystem.GetNewComponent();
				static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
					[]( KG::Core::GameObject* gameObject, float elapsedTime )
					{
						auto tran = gameObject->GetComponent<KG::Component::TransformComponent>();
						tran->RotateEuler( 0.0f, 90.0f * elapsedTime, 0.0f );
					}
				);
				auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( (x & 1) != (y & 1) ? texOne : texTwo ) );
				auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( (x & 1) !=  (y & 1) ? "sphere"_id : "cone"_id ) );
				auto* ren = this->renderer->GetNewRenderComponent();
				testCubeObjects[index].name = "meshObject0";
				testCubeObjects[index].AddComponent( tran );
				testCubeObjects[index].AddComponent( mat );
				testCubeObjects[index].AddComponent( lam );
				testCubeObjects[index].AddComponent( geo );
				testCubeObjects[index].AddComponent( ren );
				testCubeObjects[index].GetComponent<KG::Component::TransformComponent>()->Translate( x, 0.0f, y );
				testCubeObjects[index].GetComponent<KG::Component::TransformComponent>()->SetScale( DirectX::XMFLOAT3(1,1,1) * 0.25f );
			}

			{
				auto* tran = this->system->transformSystem.GetNewComponent();
				auto* light = this->renderer->GetNewLightComponent();
				testPointLightObjects[index].name = "Light2";

				auto color = Math::RandomColor();

				light->SetPointLight( DirectX::XMFLOAT3( color.x, color.y, color.z), 0.1f, 1.0f );

				testPointLightObjects[index].AddComponent( tran );
				testPointLightObjects[index].AddComponent( light );
				testPointLightObjects[index].GetComponent<KG::Component::TransformComponent>()->Translate( x, 0.0f, y );

				tran->Translate( 0.5f, 0, 0.5f );
			}

		}
	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* light = this->renderer->GetNewLightComponent();
		testLightObject.name = "Light";
		testLightObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		light->SetDirectionalLight( DirectX::XMFLOAT3( 0.1f, 0.1f, 0.1f ) * 9, DirectX::XMFLOAT3( 0.0f, -1.0f, -1.0f) );
		testLightObject.AddComponent( light );
	}


}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();
	//DebugNormalMessage( "OnUpdated");
	this->input->ProcessInput( this->engineDesc.hWnd );
	this->system->OnUpdate( this->timer.GetTimeElapsed() );
	this->renderer->Update( this->timer.GetTimeElapsed() );
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
