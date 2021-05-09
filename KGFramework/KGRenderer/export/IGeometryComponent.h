#pragma once
#include <vector>
#include <deque>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
namespace KG::Component
{
	class DLL IGeometryComponent : public IRenderComponent
	{
		virtual void OnCreate(KG::Core::GameObject* obj) override = 0;
	public:
		virtual bool HasBone() const = 0;
		virtual void AddGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0) = 0;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
	};
	REGISTER_COMPONENT_ID( IGeometryComponent );
};
