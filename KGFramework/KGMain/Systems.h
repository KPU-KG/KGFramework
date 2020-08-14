#pragma once
#include "TransformSystem.h"
#include "Systems.h"
namespace KG
{
	struct Systems
	{
		KG::System::TransformSystem transformSystem;

		void OnPreRender()
		{
			this->transformSystem.OnPreRender();
		}
		void OnUpdate( float elaspedTime )
		{
			this->transformSystem.OnUpdate( elaspedTime );
		}
		void OnPostUpdate( float elaspedTime )
		{
			this->transformSystem.OnPostUpdate( elaspedTime );
		}
	};
};