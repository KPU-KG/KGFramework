#include "pch.h"
#include "GraphicSystem.h"
#include <queue>

void KG::System::Render2DSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::Render2DSystem::OnPostUpdate(float elapsedTime)
{
}

class Render2DComp {
public:
    bool operator()(KG::Component::Render2DComponent* a, KG::Component::Render2DComponent* b) const {
        return a->transform2D.depth < b->transform2D.depth;
    }
};


void KG::System::Render2DSystem::OnPreRender()
{
    static std::priority_queue<KG::Component::Render2DComponent*, std::vector<KG::Component::Render2DComponent*>, Render2DComp> queue;
    for ( auto& com : this->pool )
    {
        queue.push(&com);
    }
    while (!queue.empty())
    {
        queue.top()->OnPreRender();
        queue.pop();
    }
}


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

void KG::System::Render3DSystem::OnProcessCulling(const DirectX::BoundingFrustum& frustum)
{
    for (Render3DComponent& com : this->pool)
    {
        com.CullingProcess(frustum);
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
	for ( auto& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::LightSystem::OnGetNewComponent( LightComponent* target )
{
}

void KG::System::LightSystem::OnUpdate( float elapsedTime )
{
}

void KG::System::LightSystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::LightSystem::OnPreRender()
{
	for ( auto& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::CubeCameraSystem::OnGetNewComponent( CubeCameraComponent* target )
{
}

void KG::System::CubeCameraSystem::OnUpdate( float elapsedTime )
{
}

void KG::System::CubeCameraSystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::CubeCameraSystem::OnPreRender()
{
	for ( auto& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::AvatarSystem::OnUpdate( float elapsedTime )
{
}

void KG::System::AvatarSystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::AvatarSystem::OnPreRender()
{
	for ( auto& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::ShadowCasterSystem::OnUpdate( float elapsedTime )
{
}

void KG::System::ShadowCasterSystem::OnPostUpdate( float elapsedTime )
{
}

void KG::System::ShadowCasterSystem::OnPreRender()
{
	for ( auto& com : this->pool )
	{
		com.OnPreRender();
	}
}

void KG::System::AnimationControllerSystem::OnUpdate(float elapsedTime)
{
	//DebugNormalMessage("Update Animation System");
	for (auto& com : this->pool)
	{
		com.Update(elapsedTime);
	}
}

void KG::System::AnimationControllerSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::AnimationControllerSystem::OnPreRender()
{
}

void KG::System::ParticleEmitterSystem::OnUpdate(float elapsedTime)
{
	for ( auto& com : this->pool )
	{
		com.Update(elapsedTime);
	}
}

void KG::System::ParticleEmitterSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::ParticleEmitterSystem::OnPreRender()
{
}

void KG::System::RenderSpriteSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::RenderSpriteSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::RenderSpriteSystem::OnPreRender()
{
    for ( auto& com : this->pool )
    {
        com.OnPreRender();
    }
}

void KG::System::PostProcessorSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::PostProcessorSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::PostProcessorSystem::OnPreRender()
{
}
