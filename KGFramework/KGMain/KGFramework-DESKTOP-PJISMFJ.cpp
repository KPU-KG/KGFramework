#include "KGFramework.h"
#include "KGRenderer.h"
#include "Systems.h"
#include "Debug.h"

#include "GameObject.h"
#include "LambdaComponent.h"


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

	//자원 미리 할당
	this->windowText.reserve( 100 );

	OnTestInit();

	return true;
}

void KG::GameFramework::OnTestInit()
{
	static KG::Core::GameObject testGameObject;
	static KG::Core::GameObject testCameraObject;

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* lam = this->system->lambdaSystem.GetNewComponent();
		static_cast<KG::Component::LambdaComponent*>(lam)->PostFunction(
			[]( KG::Core::GameObject* gameObject, float elapsedTime ) 
			{
				static bool tos = false;
				auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
				auto value = GetKeyState( VK_SPACE );
				DebugNormalMessage( "Key Input Value : " << value);
				if ( value & 0x0001 )
				{
					tos = !tos;
				}
				if(	tos )
					trans->RotateEuler(0.0f, 180.0f * elapsedTime, 0.0f);
				auto euler = trans->GetEulerAngle();
				//DebugNormalMessage( "Key Input Value : " << std::boolalpha << tos );
				DebugNormalMessage( "rotation : " << euler );
			}
		);
		auto* mat = this->renderer->GetNewMaterialComponent( KG::Utill::HashString( "deferredDefault"_id ) );
		auto* geo = this->renderer->GetNewGeomteryComponent( KG::Utill::HashString( "heli"_id ) );
		auto* ren = this->renderer->GetNewRenderComponent();
		testGameObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		testGameObject.AddComponent( mat );
		testGameObject.AddComponent( lam );
		testGameObject.AddComponent( geo );
		testGameObject.AddComponent( ren );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->Translate( 0, 0, 0.0f );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->RotateEuler( 0.0f, 0.0f, 0.0f );
		testGameObject.GetComponent<KG::Component::TransformComponent>()->SetScale( 0.01f, 0.01f, 0.01f );

	}

	{
		auto* tran = this->system->transformSystem.GetNewComponent();
		auto* cam = this->renderer->GetNewCameraComponent();
		testCameraObject.AddComponent( static_cast<KG::Component::TransformComponent*>(tran) );
		testCameraObject.AddComponent( cam );
		testCameraObject.GetComponent<KG::Component::TransformComponent>()->Translate( 0, 0, -2.0f );
	}
}

void KG::GameFramework::OnProcess()
{
	this->timer.Tick();
	this->UpdateWindowText();
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
