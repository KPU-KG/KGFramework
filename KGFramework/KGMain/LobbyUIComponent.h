#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>
namespace KG::Component
{
    class ICameraComponent;
    struct LobbyPimpl;
    class LobbyUIComponent : public IComponent
    {
    private:
        LobbyPimpl* pimpl;
        virtual void OnCreate(KG::Core::GameObject* obj) override;
    public:
        virtual void Update(float elapsedTime) override;
        virtual void OnDestroy() override;

    public:
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
    REGISTER_COMPONENT_ID(LobbyUIComponent);

    class LobbyUIComponentSystem : public KG::System::IComponentSystem<LobbyUIComponent>
    {
    public:
        virtual void OnUpdate(float elapsedTime) override
        {
            for (auto& com : *this)
            {
                com.Update(elapsedTime);
            }
        }

        // IComponentSystem을(를) 통해 상속됨
        virtual void OnPostUpdate(float elapsedTime) override
        {

        }
        virtual void OnPreRender() override
        {

        }

    };
}