#include "FakeGeometryComponent.h"

void KG::Component::FakeGeometryComponent::InitializeGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
{
    auto* inst = KG::Resource::FakeResourceContainer::GetInstance();
    if ( this->geometrys.size() < slotIndex + 1 )
    {
        this->geometrys.resize(slotIndex + 1);
    }
    this->geometrys[slotIndex] = inst->LoadGeometry(geometryID, subMeshIndex);
}

void KG::Component::FakeGeometryComponent::OnCreate(KG::Core::GameObject* obj)
{
    IRenderComponent::OnCreate(obj);
    auto* inst = KG::Resource::FakeResourceContainer::GetInstance();
    for ( auto& i : this->geometryDescs.materialDescs )
    {
        this->InitializeGeometry(i.geometryID, i.subMeshIndex, i.slotIndex);
    }
}

bool KG::Component::FakeGeometryComponent::HasBone() const
{
    bool result = false;
    for ( auto* i : geometrys )
    {
        if ( i->HasBone() )
            return true;
    }
    return false;
}

void KG::Component::FakeGeometryComponent::AddGeometry(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
{
    this->geometryDescs.materialDescs.emplace_back(
        geometryID, subMeshIndex, slotIndex
    );
}

void KG::Component::FakeGeometryComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
    this->geometryDescs.OnDataLoad(componentElement);
}

void KG::Component::FakeGeometryComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeGeometryComponent::OnDrawGUI()
{
    return false;
}

KG::Component::FakeGeometryDesc::FakeGeometryDesc()
    :geometryIDProp("geometryID", geometryID),
    subMeshIndexProp("subMeshIndex", subMeshIndex),
    slotIndexProp("slotIndex", slotIndex)
{
}

KG::Component::FakeGeometryDesc::FakeGeometryDesc(const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex)
    : FakeGeometryDesc()
{
    this->geometryID = geometryID;
    this->subMeshIndex = subMeshIndex;
    this->slotIndex = slotIndex;
}

KG::Component::FakeGeometryDesc::FakeGeometryDesc(const FakeGeometryDesc& other)
    : FakeGeometryDesc()
{
    this->geometryID = other.geometryID;
    this->subMeshIndex = other.subMeshIndex;
    this->slotIndex = other.slotIndex;
}

KG::Component::FakeGeometryDesc::FakeGeometryDesc(FakeGeometryDesc&& other)
    : FakeGeometryDesc()
{
    this->geometryID = other.geometryID;
    this->subMeshIndex = other.subMeshIndex;
    this->slotIndex = other.slotIndex;
}

KG::Component::FakeGeometryDesc& KG::Component::FakeGeometryDesc::operator=(const FakeGeometryDesc& other)
{
    this->geometryID = other.geometryID;
    this->subMeshIndex = other.subMeshIndex;
    this->slotIndex = other.slotIndex;
    return *this;
}

KG::Component::FakeGeometryDesc& KG::Component::FakeGeometryDesc::operator=(FakeGeometryDesc&& other)
{
    this->geometryID = other.geometryID;
    this->subMeshIndex = other.subMeshIndex;
    this->slotIndex = other.slotIndex;
    return *this;
}

void KG::Component::FakeGeometryDescs::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
    auto* obj = objectElement->FirstChildElement("GeometryDescs");
    auto* nextElement = obj->FirstChildElement("Element");
    while ( nextElement != nullptr )
    {
        FakeGeometryDesc desc;
        desc.geometryIDProp.OnDataLoad(nextElement);
        desc.slotIndexProp.OnDataLoad(nextElement);
        desc.subMeshIndexProp.OnDataLoad(nextElement);
        this->materialDescs.emplace_back(desc);
        nextElement = nextElement->NextSiblingElement("Element");
    }
}

void KG::Component::FakeGeometryDescs::OnDataSave(tinyxml2::XMLElement* objectElement)
{
}

bool KG::Component::FakeGeometryDescs::OnDrawGUI()
{
    return false;
}
