#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IMaterialComponent.h"

namespace KG::Component
{
	class FakeMaterialComponent : public IMaterialComponent
	{
        // IMaterialComponent을(를) 통해 상속됨
        virtual void InitializeMaterial(const KG::Utill::HashString& materialID, UINT slotIndex = 0) override;
        virtual void InitializeShader(const KG::Utill::HashString& shaderID, UINT slotIndex = 0) override;
        virtual void OnDestroy() override;
        virtual void OnCreate(KG::Core::GameObject* obj) override;
        virtual void PostMaterial(const KG::Utill::HashString& materialID, UINT slotIndex = 0) override;
        virtual void PostShader(const KG::Utill::HashString& shaderID, UINT slotIndex = 0) override;
        virtual void ReloadMaterial() override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
	REGISTER_COMPONENT_ID_REPLACE( FakeMaterialComponent, IMaterialComponent);
};
