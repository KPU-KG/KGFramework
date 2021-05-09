#pragma once
#include <vector>
#include <deque>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IGeometryComponent.h"
#include "FakeResourceContainer.h"

namespace KG::Component
{
    struct FakeGeometryComponent;
    struct FakeGeometryDescs;

    struct FakeGeometryDesc
    {
        friend KG::Component::FakeGeometryComponent;
        friend KG::Component::FakeGeometryDescs;
        KG::Utill::HashString geometryID = 0;
        UINT subMeshIndex = 0;
        UINT slotIndex = 0;
    public:
        FakeGeometryDesc();
        FakeGeometryDesc(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex);
        FakeGeometryDesc(const FakeGeometryDesc& other);
        FakeGeometryDesc(FakeGeometryDesc&& other);
        FakeGeometryDesc& operator=(const FakeGeometryDesc& other);
        FakeGeometryDesc& operator=(FakeGeometryDesc&& other);
    private:
        KG::Core::SerializableProperty<KG::Utill::HashString> geometryIDProp;
        KG::Core::SerializableProperty<UINT> subMeshIndexProp;
        KG::Core::SerializableProperty<UINT> slotIndexProp;

    };

    struct FakeGeometryDescs : public KG::Core::ISerializable
    {
        std::vector<FakeGeometryDesc> materialDescs;
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

	class FakeGeometryComponent : public IGeometryComponent
	{
    public:
        FakeGeometryDescs geometryDescs;
        std::vector<KG::Resource::FakeGeometry*> geometrys;
        void InitializeGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex);
        // IGeometryComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* obj) override;
        virtual bool HasBone() const override;
        virtual void AddGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex = 0, UINT slotIndex = 0) override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
    };
    REGISTER_COMPONENT_ID_REPLACE( FakeGeometryComponent, IGeometryComponent);
};
