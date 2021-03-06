#pragma once
#include "TransformSystem.h"
#include "LambdaComponent.h"
#include "SceneCameraComponent.h"
#include "PlayerControllerComponent.h"
#include "LobbyUIComponent.h"
#include "Systems.h"
namespace KG
{
	struct Systems
	{
		KG::System::TransformSystem transformSystem;
		KG::System::LambdaComponentSystem lambdaSystem;
		KG::System::SceneCameraComponentSystem sceneCameraSystem;
        KG::System::PlayerControllerComponentSystem playerComponentSystem;
        KG::System::LobbyUIComponentSystem lobbyComponentSystem;

		void PostComponentProvider(KG::Component::ComponentProvider& provider)
		{
			this->lambdaSystem.OnPostProvider(provider);
			this->transformSystem.OnPostProvider(provider);
			this->sceneCameraSystem.OnPostProvider(provider);
			this->playerComponentSystem.OnPostProvider(provider);
            this->lobbyComponentSystem.OnPostProvider(provider);
        }

		void OnPreRender()
		{
			this->transformSystem.OnPreRender();
		}
		void OnUpdate(float elaspedTime)
		{
			this->lambdaSystem.OnUpdate(elaspedTime);
			this->transformSystem.OnUpdate(elaspedTime);
			this->sceneCameraSystem.OnUpdate(elaspedTime);
			this->playerComponentSystem.OnUpdate(elaspedTime);
            this->lobbyComponentSystem.OnUpdate(elaspedTime);

		}
		void OnPostUpdate(float elaspedTime)
		{
			this->lambdaSystem.OnPostUpdate(elaspedTime);
			this->transformSystem.OnPostUpdate(elaspedTime);
			this->sceneCameraSystem.OnPostUpdate(elaspedTime);
			this->playerComponentSystem.OnPostUpdate(elaspedTime);
            this->lobbyComponentSystem.OnPostUpdate(elaspedTime);

		}
	};
};