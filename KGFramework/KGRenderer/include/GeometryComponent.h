#pragma once
#include <vector>
#include <deque>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IGeometryComponent.h"
#include "IDXRenderComponent.h"

namespace KG::Renderer
{
	class Geometry;
};

namespace KG::Component
{
	class Render3DComponent;
	class BoneTransformComponent;
	class GeometryComponent;
	struct GeometryDesc;
	struct GeometryDescs;

	struct GeometryDesc
	{
		friend KG::Component::GeometryComponent;
		friend KG::Component::GeometryDescs;
		KG::Utill::HashString geometryID = 0;
		UINT subMeshIndex = 0;
		UINT slotIndex = 0;
	public:
		GeometryDesc();
		GeometryDesc(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex);
		GeometryDesc(const GeometryDesc& other);
		GeometryDesc(GeometryDesc&& other);
		GeometryDesc& operator=(const GeometryDesc& other);
		GeometryDesc& operator=(GeometryDesc&& other);
	private:
		KG::Core::SerializableProperty<KG::Utill::HashString> geometryIDProp;
		KG::Core::SerializableProperty<UINT> subMeshIndexProp;
		KG::Core::SerializableProperty<UINT> slotIndexProp;

	};

	struct GeometryDescs : public KG::Core::ISerializable	
	{
		std::vector<GeometryDesc> materialDescs;
		// ISerializable을(를) 통해 상속됨
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;

		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;

		virtual bool OnDrawGUI() override;

		auto size() const
		{
			return this->materialDescs.size();
		}

		auto& operator[](size_t index)
		{
			return this->materialDescs[index];
		}
	};

	class GeometryComponent : public IGeometryComponent, IDXRenderComponent
	{
		friend Render3DComponent;
		friend BoneTransformComponent;
		std::vector<KG::Renderer::Geometry*> geometrys;
		void InitializeGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex);
	public:
		GeometryDescs geometryDescs;
		GeometryComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual bool HasBone() const override;
		//Serialize Part
	public:
		virtual void AddGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0) override;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID( GeometryComponent );
};
