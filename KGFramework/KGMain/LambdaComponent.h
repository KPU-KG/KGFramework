#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>
namespace KG::Component
{


	class LambdaComponent : public IComponent
	{
	private:
		using UpdateFunctor = std::function<void( KG::Core::GameObject*, float )>;
		UpdateFunctor updateFunctor;
	public:
		virtual void Update( float elapsedTime ) override
		{
			DebugAssertion( updateFunctor, L"람다 함수가 등록되지 않았습니다." );
			this->updateFunctor( this->gameObject, elapsedTime );
		}
		virtual void PostUpdateFunction( const UpdateFunctor& functor )
		{
			this->updateFunctor = functor;
		}
		virtual void OnDestroy() override
		{
			this->updateFunctor = nullptr;
			IComponent::OnDestroy();
		}
	};
	REGISTER_COMPONENT_ID( LambdaComponent );

	class LambdaComponentSystem : public KG::System::IComponentSystem<LambdaComponent>
	{
	public:
		virtual void OnUpdate( float elapsedTime ) override
		{
			for ( auto& com : *this )
			{
				com.Update( elapsedTime );
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;

	};
}