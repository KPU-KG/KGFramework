#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include <functional>
namespace KG::Component
{


	class LambdaComponent : public IComponent
	{
	private:
		using Functor = std::function<void( KG::Core::GameObject*, float )>;
		Functor functor;
	public:
		void OnUpdate( float elapsedTime )
		{
			this->functor( this->gameObject , elapsedTime );
		}
		void PostFunction( const std::function<void( KG::Core::GameObject* , float )>& functor )
		{
			this->functor = functor;
		}
		virtual void OnDestroy() override
		{
			this->functor = nullptr;
			IComponent::OnDestroy();
		}
	};
	REGISTER_COMPONENT_ID( LambdaComponent );

	class LambdaComponentSystem : public KG::System::IComponentSystem<LambdaComponent>
	{
	public:
		void OnUpdate( float elapsedTime )
		{
			for ( auto& com : *this )
			{
				com.OnUpdate( elapsedTime );
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;
	};
}