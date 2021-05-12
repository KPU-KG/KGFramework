#include "FakeSystems.h"

void KG::System::FakeGeometrySystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeRender3DSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeMaterialSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeCameraSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeCubeCameraSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeLightSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeAvatarSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeAnimationControllerSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeShadowCasterSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeParticleEmitterSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}

void KG::System::FakeRender2DSystem::OnPreRender()
{
    for ( auto& i : this->pool )
    {
        i.OnPreRender();
    }
}
