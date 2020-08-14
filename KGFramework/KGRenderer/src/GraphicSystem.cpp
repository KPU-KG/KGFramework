#include "pch.h"
#include "GraphicSystem.h"

void KG::System::Render3DSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::Render3DSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::Render3DSystem::OnPreRender()
{
	for ( Render3DComponent& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::MaterialSystem::OnGetNewComponent(MaterialComponent* target)
{
}

void KG::System::MaterialSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::MaterialSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::MaterialSystem::OnPreRender()
{
}

void KG::System::GeometrySystem::OnUpdate( float elapsedTime )
{
}

void KG::System::GeometrySystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::GeometrySystem::OnPreRender()
{
}

void KG::System::CameraSystem::OnGetNewComponent( CameraComponent* target )
{
}

void KG::System::CameraSystem::OnUpdate( float elapsedTime )
{
}

void KG::System::CameraSystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::CameraSystem::OnPreRender()
{
}
