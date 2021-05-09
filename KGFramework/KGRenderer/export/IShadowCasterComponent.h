#pragma once
#include <DirectXMath.h>
#include "IRenderComponent.h"
#include "ILightComponent.h"

namespace KG::Component
{
	class DLL IShadowCasterComponent : public IRenderComponent
	{
		virtual void OnCreate( KG::Core::GameObject* gameObject ) = 0;
		virtual void OnDestroy() = 0 ;
	public:
		virtual void OnPreRender() override = 0;
		virtual bool isPointLightShadow() const = 0;
		virtual bool isDirectionalLightShadow() const = 0;
		virtual bool isSpotLightShadow() const = 0;

		virtual KG::Component::LightType GetTargetLightType() const = 0;

		//Serialize Part
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
	};
	REGISTER_COMPONENT_ID(IShadowCasterComponent);
};
