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
			DebugAssertion( updateFunctor, L"���� �Լ��� ��ϵ��� �ʾҽ��ϴ�." );
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

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate( float elapsedTime ) override;
		virtual void OnPreRender() override;

	};
}