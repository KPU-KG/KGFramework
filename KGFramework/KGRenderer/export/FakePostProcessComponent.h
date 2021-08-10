#pragma once
#include <vector>
#include <DirectXMath.h>
#include "KGColor.h"
#include "MathHelper.h"
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IPostProcessComponent.h"

namespace KG::Component
{
    class FakePostProcessManagerComponent : public IPostProcessManagerComponent
    {
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
    public:
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
        virtual bool OnDrawGUI();
    };

    REGISTER_COMPONENT_ID_REPLACE(FakePostProcessManagerComponent, IPostProcessManagerComponent);
};
