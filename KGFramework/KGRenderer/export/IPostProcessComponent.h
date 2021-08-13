#pragma once
#include <vector>
#include <DirectXMath.h>
#include "KGColor.h"
#include "MathHelper.h"
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Component
{
    class DLL IPostProcessManagerComponent : public IRenderComponent
    {
        virtual void OnCreate(KG::Core::GameObject* gameObject) override = 0;
    public:
        virtual void SetActive(int index, bool isActive) = 0;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
        virtual bool OnDrawGUI() = 0;
    };
    REGISTER_COMPONENT_ID(IPostProcessManagerComponent);
};
