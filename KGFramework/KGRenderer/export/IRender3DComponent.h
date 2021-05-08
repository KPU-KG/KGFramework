#pragma once
#include <vector>
#include "IRenderComponent.h"

namespace KG::Component
{
	class ICubeCameraComponent;

	class DLL IRender3DComponent : public IRenderComponent
	{
		virtual void OnCreate( KG::Core::GameObject* gameObject ) override = 0;
	public:
		virtual void OnPreRender() override = 0;
		virtual void SetVisible( bool visible ) = 0;
        virtual bool GetVisible() const = 0;
		virtual void SetReflectionProbe( ICubeCameraComponent* probe ) = 0;
		virtual void ReloadRender() = 0;
		//Serialize Part
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};
	REGISTER_COMPONENT_ID(IRender3DComponent);
};
