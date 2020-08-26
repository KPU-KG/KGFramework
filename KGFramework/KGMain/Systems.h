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

		void OnPreRender()
		{
			this->transformSystem.OnPreRender();
		}
		void OnUpdate( float elaspedTime )
		{
			this->transformSystem.OnUpdate( elaspedTime );
			this->lambdaSystem.OnUpdate( elaspedTime );
		}
		void OnPostUpdate( float elaspedTime )
		{
			this->transformSystem.OnPostUpdate( elaspedTime );
			this->lambdaSystem.OnPostUpdate( elaspedTime );
		}
	};
};