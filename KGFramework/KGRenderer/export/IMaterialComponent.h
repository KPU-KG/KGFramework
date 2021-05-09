#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Component
{
	class DLL IMaterialComponent : public IRenderComponent
	{
		virtual void InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex = 0 ) = 0;
		virtual void InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex = 0 ) = 0;
		virtual void OnDestroy() override = 0;
	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override = 0;
		virtual void PostMaterial(const KG::Utill::HashString& materialID, UINT slotIndex = 0) = 0;
		virtual void PostShader(const KG::Utill::HashString& shaderID, UINT slotIndex = 0) = 0;
		virtual void ReloadMaterial() = 0;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};
	REGISTER_COMPONENT_ID( IMaterialComponent );
};
