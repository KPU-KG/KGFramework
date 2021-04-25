#pragma once
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "ServerEnemyControllerComponent.h"
#include "ServerGameManagerComponent.h"
#include "ServerPlayerControllerComponent.h"
namespace KG::System
{
	template<typename Ty>
	class SBaseComponentSystem : public KG::System::IComponentSystem<Ty>
	{
	protected:
		KG::Server::Server* server;
		virtual void OnGetNewComponent(Ty* comp) override
		{
			comp->SetServerInstance(this->server);
		}
	public:
		void SetServerInstance(KG::Server::Server* server)
		{
			this->server = server;
		}
		virtual void OnPostProvider(KG::Component::ComponentProvider& provider) override
		{
			provider.PostInjectionFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()),
				[this](KG::Core::GameObject* object)
				{
					auto* comp = this->GetNewComponent();
					comp->SetServerInstance(this->server);
					object->AddComponent<Ty>(comp);
					return comp;
				}
			);
			provider.PostGetterFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()),
				[this]()->KG::Component::IComponent*
				{
					auto* comp = this->GetNewComponent();
					comp->SetServerInstance(this->server);
					return static_cast<KG::Component::IComponent*>(comp);
				}
			);
	
		}
	};

	class SEnemyControllerComponentSystem : public KG::System::SBaseComponentSystem<SEnemyControllerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
	class SGameManagerComponentSystem : public KG::System::SBaseComponentSystem<SGameManagerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	class SPlayerComponentSystem : public KG::System::SBaseComponentSystem<SPlayerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}