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
	static KG::Core::GameObject testCubeCameraObject;
	static KG::Core::GameObject testLightObject;
	static KG::Core::GameObject testPointLightObjects[10000];
	static KG::Core::GameObject testSpotLightObjects;
	static KG::Core::GameObject testSkyObject;
	static KG::Core::GameObject testAmbientObject;
	static KG::Core::GameObject testAnimationObject;

	constexpr auto texOne = "PBRTile"_id;
	constexpr auto texTwo = "PBRTile"_id;
	constexpr auto texThree = "PBRTile"_id;
	KG::Component::CameraComponent* mainCamera = nullptr;

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
		mainCamera = cam;
		cam->SetFarZ( 50.0f );
		//프레임워크에서 카메라 세팅 // 
		KG::Renderer::RenderTextureDesc renderTextureDesc;
		renderTextureDesc.useDeferredRender = true;
		renderTextureDesc.useCubeRender = false;
		renderTextureDesc.useDepthStencilBuffer = true;
		renderTextureDesc.useRenderTarget = true;
		renderTextureDesc.width = this->setting.clientWidth;
		renderTextureDesc.height = this->setting.clientHeight;
		cam->InitializeRenderTexture( renderTextureDesc );
		cam->SetDefaultRender();
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
				auto worldPos = trans->GetWorldPosition();
				DebugNormalMessage( "LambdaTransform : " << worldPos);
			}
		);
		testCameraObject.name = "camera";
		testCameraObject.AddComponent( tran );
		testCameraObject.AddComponent( cam );
		testCameraObject.AddComponent( lam );
		testCameraObject.GetComponent<KG::Component::TransformComponent>()->Translate( 0, 0, -2.0f );
	}



	constexpr size_t cdas = 3;
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
						using namespace KG::Input;
						auto input = InputManager::GetInputManager();
						auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
						if ( input->IsTouching( VK_SPACE ) )
						{
							trans->RotateEuler( 0.0f, 90.0f * elapsedTime, 0.0f );
						}
					}
				);
				auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( (x & 1) != (y & 1) ? texThree : texTwo ) );
				auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( (x & 1) != (y & 1) ? "sphere"_id : "cube"_id ) );
				//auto* evn = this->renderer->GetNewCubeCameraComponent();
				KG::Renderer::RenderTextureDesc renderTextureDesc;
				renderTextureDesc.useDeferredRender = true;
				renderTextureDesc.useCubeRender = true;
				renderTextureDesc.useDepthStencilBuffer = true;
				renderTextureDesc.useRenderTarget = true;
				renderTextureDesc.uploadSRVRenderTarget = true;
				renderTextureDesc.width = 128;
				renderTextureDesc.height = 128;
				//evn->InitializeRenderTexture( renderTextureDesc );
				auto* ren = this->renderer->GetNewRenderComponent();
				testCubeObjects[index].name = "meshObject0";
				testCubeObjects[index].AddComponent( tran );
				testCubeObjects[index].AddComponent( mat );
				testCubeObjects[index].AddComponent( lam );
				testCubeObjects[index].AddComponent( geo );
				//testCubeObjects[index].AddComponent( evn );
				testCubeObjects[index].AddComponent( ren );
				testCubeObjects[index].GetComponent<KG::Component::TransformComponent>()->Translate( x * 2, 0.0f, y * 2 );
				testCubeObjects[index].GetComponent<KG::Component::TransformComponent>()->SetScale( DirectX::XMFLOAT3( 1, 1, 1 ) * 0.5f );
				//testCubeObjects[index].GetComponent<KG::Component::Render3DComponent>()->SetReflectionProbe( evn );
			}

		}
	}

	{
		int index = 0;
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* light = this->renderer->GetNewLightComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		auto* sdw = this->renderer->GetNewShadowCasterComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[light]( KG::Core::GameObject* gameObject, float elapsedTime )
			{
				static float str = 9.0f;
				using namespace KG::Input;
				auto input = InputManager::GetInputManager();
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				if ( input->IsTouching( VK_OEM_6 ) )
				{
					DebugNormalMessage( "Light Intense Up" );
					light->GetPointLightRef().Strength = light->GetDirectionalLightRef().Strength * 1.1f;
				}
				if ( input->IsTouching( VK_OEM_4 ) )
				{
					DebugNormalMessage( "Light Intense Down" );
					light->GetPointLightRef().Strength = light->GetDirectionalLightRef().Strength * 0.9f;
				}
			}
		);
		testPointLightObjects[index].name = "Light2";


		light->SetPointLight( DirectX::XMFLOAT3( 20.0f, 0.0f, 0.0f ), 0.1f, 10.0f );

		testPointLightObjects[index].AddComponent( tran );
		testPointLightObjects[index].AddComponent( light );
		testPointLightObjects[index].AddComponent( sdw );
		testPointLightObjects[index].AddComponent( lam );
		testPointLightObjects[index].GetComponent<KG::Component::TransformComponent>()->Translate( 0.0f, 0.0f, 0.0f );

		tran->Translate( 0.5f, 0, 0.5f );
	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* light = this->renderer->GetNewLightComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		auto* sdw = this->renderer->GetNewShadowCasterComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[light]( KG::Core::GameObject* gameObject, float elapsedTime )
			{
				static float str = 9.0f;
				using namespace KG::Input;
				auto input = InputManager::GetInputManager();
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();

			}
		);
		testSpotLightObjects.name = "Light2";


		light->SetSpotLight( DirectX::XMFLOAT3( 0.0f, 0.0f, 10.0f ), 10.0f,
			DirectX::XMConvertToRadians( 120.0f ), DirectX::XMConvertToRadians( 30.0f ), 4.0f );

		testSpotLightObjects.AddComponent( tran );
		testSpotLightObjects.AddComponent( light );
		testSpotLightObjects.AddComponent( sdw );
		testSpotLightObjects.AddComponent( lam );
		testSpotLightObjects.GetComponent<KG::Component::TransformComponent>()->Translate( 0.0f, 0.0f, -1.5f );

		//tran->Translate( 0.5f, 2.0f, 0.5f );
		//mainCamera->GetGameObject()->GetTransform()->AddChild( testSpotLightObjects.GetTransform() );
	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* light = this->renderer->GetNewLightComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		KG::Component::ShadowCasterComponent* sdw = this->renderer->GetNewShadowCasterComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[light]( KG::Core::GameObject* gameObject, float elapsedTime )
			{
				static float str = 2.0f;
				using namespace KG::Input;
				auto input = InputManager::GetInputManager();
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				if ( input->IsTouching( VK_OEM_6 ) )
				{
					DebugNormalMessage( "Light Intense Up" );
					str += 5.0f * elapsedTime;
					light->GetDirectionalLightRef().Strength = DirectX::XMFLOAT3( 0.1f, 0.1f, 0.1f ) * str;
				}
				if ( input->IsTouching( VK_OEM_4 ) )
				{
					DebugNormalMessage( "Light Intense Down" );
					str -= 5.0f * elapsedTime;
					light->GetDirectionalLightRef().Strength = DirectX::XMFLOAT3( 0.1f, 0.1f, 0.1f ) * str;
				}
			}
		);
		testLightObject.name = "Light";
		testLightObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		light->SetDirectionalLight( DirectX::XMFLOAT3( 0.1f, 0.1f, 0.1f ) * 1, DirectX::XMFLOAT3( 0.0f, -1.0f, -1.0f ) );
		sdw->SetTargetCameraCamera( mainCamera );
		testLightObject.AddComponent( light );
		testLightObject.AddComponent( sdw );
		testLightObject.AddComponent( lam );

	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* mat = this->renderer->GetNewMaterialComponentFromShader( KG::Utill::HashString( "AmbientLight" ) );
		auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( "lightPlane"_id ) );
		auto* ren = this->renderer->GetNewRenderComponent();
		testSkyObject.name = "plane";
		testSkyObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		testSkyObject.AddComponent( mat );
		testSkyObject.AddComponent( geo );
		testSkyObject.AddComponent( ren );
	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( "SkySnow" ) );
		auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( "cube"_id ) );
		auto* ren = this->renderer->GetNewRenderComponent();
		testAmbientObject.name = "plane";
		testAmbientObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		testAmbientObject.AddComponent( mat );
		testAmbientObject.AddComponent( geo );
		testAmbientObject.AddComponent( ren );

	}
	this->renderer->SetSkymapTextureId( KG::Utill::HashString( "skySnow" ) );

	{
		static KG::Core::ObjectContainer oc;
		oc.PostTransformSystem( &this->system->transformSystem );
		KG::Resource::MaterialMatch match;

		match.SetDefaultMaterial( { KG::Utill::HashString( "soldierHead"_id ), KG::Utill::HashString( "soldierBody"_id ) } );
		auto* ptr = this->renderer->LoadFromModel( KG::Utill::HashString( "soldier"_id ), oc, match );
		ptr->GetComponent<KG::Component::TransformComponent>()->SetScale( 0.01f, 0.01f, 0.01f );
		ptr->GetComponent<KG::Component::TransformComponent>()->SetPosition( 0.5f, -0.5f, 0.0f );
		ptr->name = "soldier";

		auto* ctrl = this->renderer->GetNewAnimationControllerComponent();

		ctrl->RegisterAnimation(KG::Utill::HashString("soldier_sprint_forward"_id));
		ctrl->RegisterAnimation(KG::Utill::HashString("soldier_walk_forward"_id));
		ctrl->RegisterAnimation(KG::Utill::HashString("soldier_standing"_id));
		ctrl->RegisterAnimation(KG::Utill::HashString("soldier_walk_right"_id));
		ctrl->RegisterAnimation(KG::Utill::HashString("soldier_walk_left"_id));

		ctrl->SetAnimation(KG::Utill::HashString("soldier_walk_forward"_id));
		ctrl->SetDefaultAnimation(KG::Utill::HashString("soldier_walk_forward"_id));
		ptr->AddComponent(ctrl);

		auto* lam = this->system->lambdaSystem.GetNewComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostUpdateFunction(
			[ctrl](KG::Core::GameObject* gameObject, float elapsedTime)
			{
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				using namespace KG::Input;
				auto input = InputManager::GetInputManager();
				
				if (input->GetKeyState('1') == KG::Input::KeyState::Down)
				// if (input->IsTouching('1'))
				{
					// -1 : 무한 루프
					ctrl->ChangeAnimation(KG::Utill::HashString("soldier_walk_left"_id), ANIMSTATE_PLAYING, 0.2f, ANIMLOOP_INF);
				}
				if (input->GetKeyState('2') == KG::Input::KeyState::Down)
				// if (input->IsTouching('2'))
				{
					ctrl->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), ANIMSTATE_PLAYING, 0.2f, ANIMLOOP_INF);
				}
				if (input->GetKeyState('3') == KG::Input::KeyState::Down)
				// if (input->IsTouching('3'))
				{
					ctrl->ChangeAnimation(KG::Utill::HashString("soldier_walk_right"_id), ANIMSTATE_PLAYING, 0.2f, ANIMLOOP_INF);
				}
				if (input->GetKeyState('4') == KG::Input::KeyState::Down)
				// if (input->IsTouching('4'))
				{
					ctrl->ChangeAnimation(KG::Utill::HashString("soldier_walk_forward"_id), ANIMSTATE_PLAYING, 0.2f, ANIMLOOP_INF);
					ctrl->BlendingAnimation(KG::Utill::HashString("soldier_walk_right"_id), ANIMLOOP_INF, ANIMINDEX_CHANGE);
				}
			}
		);
		ptr->AddComponent(lam);
	}
}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();
	DebugNormalMessage( "OnUpdatedProcess");
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
