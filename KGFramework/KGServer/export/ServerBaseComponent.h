#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


namespace KG::Server
{
	class IServer;
};
namespace KG::Component
{
	class DLL SBaseComponent : public IComponent
	{
	protected:
		KG::Server::IServer* server;
	public:
		void SetServerInstance(KG::Server::IServer* server)
		{
			this->server = server;
		}
	};

	template<typename Ty>
	class DLL SBaseComponentSystem : public KG::System::IComponentSystem<Ty>
	{
	protected:
		KG::Server::IServer* server;
	public:
		void SetServerInstance(KG::Server::IServer* server)
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

}