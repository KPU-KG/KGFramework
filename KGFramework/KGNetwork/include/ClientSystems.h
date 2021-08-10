#pragma once
#include "IComponent.h"
#include "ISystem.h"

#include "ClientGameManagerComponent.h"
#include "ClientCharacterComponent.h"
#include "ClientEnemyControllerComponent.h"
#include "ClientPlayerControllerComponent.h"
#include "ClientLobbyComponent.h"
#include "ClientProjectileComponent.h"

namespace KG::System
{
	class CGameManagerComponentSystem : public KG::System::IComponentSystem<CGameManagerComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CGameManagerComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	class CEnemyControllerComponentSystem : public KG::System::IComponentSystem<CEnemyControllerComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CEnemyControllerComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}
	
		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	class CProjectileComponentSystem : public KG::System::IComponentSystem<CProjectileComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CEnemyControllerComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for (auto& com : *this)
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	class CPlayerControllerComponentSystem : public KG::System::IComponentSystem<CPlayerControllerComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CPlayerControllerComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
	class CCharacterComponentSystem : public KG::System::IComponentSystem<CCharacterComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CCharacterComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	class CLobbyComponentSystem : public KG::System::IComponentSystem<CLobbyComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CLobbyComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for (auto& com : *this)
			{
				com.Update(elapsedTime);
			}
		}
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}