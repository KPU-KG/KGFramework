#pragma once
#include "TransformSystem.h"
#include "LambdaComponent.h"
#include "Systems.h"
namespace KG
{
	struct Systems
	{
		KG::System::TransformSystem transformSystem;
		KG::System::LambdaComponentSystem lambdaSystem;

		void PostComponentProvider(KG::Component::ComponentProvider& provider)
		{
			this->lambdaSystem.OnPostProvider(provider);
			this->transformSystem.OnPostProvider(provider);
		}

		void OnPreRender()
		{
			this->transformSystem.OnPreRender();
		}
		void OnUpdate( float elaspedTime )
		{
			this->lambdaSystem.OnUpdate( elaspedTime );
			this->transformSystem.OnUpdate( elaspedTime );
		}
		void OnPostUpdate( float elaspedTime )
		{
			this->lambdaSystem.OnPostUpdate( elaspedTime );
			this->transformSystem.OnPostUpdate( elaspedTime );
		}
	};
};