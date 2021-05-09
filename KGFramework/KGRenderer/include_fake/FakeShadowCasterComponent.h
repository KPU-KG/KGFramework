#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"
#include "IShadowCasterComponent.h"

namespace KG::Component
{
	class FakeShadowCasterComponent : public IShadowCasterComponent
	{
    public:
        // IShadowCasterComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnDestroy() override;
        virtual void OnPreRender() override;
        virtual bool isPointLightShadow() const override;
        virtual bool isDirectionalLightShadow() const override;
        virtual bool isSpotLightShadow() const override;
        virtual KG::Component::LightType GetTargetLightType() const override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
    };
    REGISTER_COMPONENT_ID_REPLACE( FakeShadowCasterComponent, IShadowCasterComponent);
};
