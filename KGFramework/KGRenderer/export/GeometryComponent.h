#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Renderer
{
	class Geometry;
};

namespace KG::Component
{
	class Render3DComponent;
	class BoneTransformComponent;

	class DLL GeometryComponent : public IRenderComponent
	{
		friend Render3DComponent;
		friend BoneTransformComponent;
		std::vector<KG::Renderer::Geometry*> geometrys;
	public:
		KG::Utill::HashString geometryID = 0;
		UINT subMeshIndex = 0;
		UINT slotIndex = 0;
		GeometryComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		bool HasBone() const;
		//Serialize Part
	private:
		KG::Core::SerializableProperty<KG::Utill::HashString> geometryIDProp;
		KG::Core::SerializableProperty<UINT> subMeshIndexProp;
		KG::Core::SerializableProperty<UINT> slotIndexProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID( GeometryComponent );
};
