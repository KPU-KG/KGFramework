#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "IRender3DComponent.h"
namespace KG::Component
{

	class FakeRender3DComponent : public IRender3DComponent
	{
    public:
        // IRender3DComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnPreRender() override;
        virtual void SetVisible(bool visible) override;
        virtual bool GetVisible() const override;
        virtual void SetReflectionProbe(ICubeCameraComponent* probe) override;
        virtual void ReloadRender() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
    REGISTER_COMPONENT_ID_REPLACE(FakeRender3DComponent, IRender3DComponent);
};
