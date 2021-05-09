#include "pch.h"
#include <map>
#include <mutex>
#include <algorithm>
#include <stack>

#include <future>

#include "Debug.h"
#include "Scene.h"
#include "FakeResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "KGResourceLoader.h"
#include "KGFakeRenderer.h"
#include "KGModel.h"
#include "Transform.h"
#include "ImguiHelper.h"

std::unique_ptr<KG::Resource::FakeResourceContainer> KG::Resource::FakeResourceContainer::instance = nullptr;

KG::Resource::FakeResourceContainer::FakeResourceContainer()
{
    DebugNormalMessage(TOSTRING(FakeResourceContainer) << " : Initialize");
}

KG::Resource::FakeResourceContainer::~FakeResourceContainer()
{
    DebugNormalMessage(TOSTRING(FakeResourceContainer) << " : Removed");
}

KG::Resource::FrameModel* KG::Resource::FakeResourceContainer::LoadModel(const KG::Utill::HashString& id)
{
    if ( this->models.count(id) )
    {
        return &this->models.at(id);
    }
    else if ( this->preloadModels.count(id) )
    {
        auto* model = &this->models.emplace(id, this->preloadModels[id].get()).first->second;
        auto& frame = model->data;
        for ( size_t i = 0; i < frame.meshs.size(); i++ )
        {
            this->CreateGeometry(id, i, frame.meshs[i]);
        }
        return model;
    }
    else
    {
        auto metaData = ResourceLoader::LoadGeometrySetFromFile("Resource/GeometrySet.xml", id);
        auto* model = &this->models.emplace(id, metaData).first->second;
        auto& frame = model->data;
        for ( size_t i = 0; i < frame.meshs.size(); i++ )
        {
            this->CreateGeometry(id, i, frame.meshs[i]);
        }
        return model;
    }
}

//프리로드 루틴 -> 프로그램 시작되면 프로미스 걸고 컨테이너에 퓨처 넣은 후 비동기 로딩
// 실제 로드 모델 불리우면 퓨처 리스트에 있나 확인
//없으면 그냥 로딩
// 있으면 get으로 받아옴 

static void AsyncLoadFrameModel(std::vector<KG::Utill::HashString>&& vectors, std::vector<std::promise<KG::Resource::FrameModel>>&& promises)
{
    for ( size_t i = 0; i < vectors.size(); i++ )
    {
        auto metaData = KG::Resource::ResourceLoader::LoadGeometrySetFromFile("Resource/GeometrySet.xml", vectors[i]);
        promises[i].set_value(KG::Resource::FrameModel(metaData));
    }
}

void KG::Resource::FakeResourceContainer::PreLoadModels(std::vector<KG::Utill::HashString>&& vectors)
{
    DebugNormalMessage("Preload Models Start");
    std::vector<std::promise<KG::Resource::FrameModel>> promises;
    promises.resize(vectors.size());
    for ( size_t i = 0; i < vectors.size(); i++ )
    {
        this->preloadModels.emplace(vectors[i], promises[i].get_future());
    }
    std::thread preloadThread{ AsyncLoadFrameModel , std::move(vectors), std::move(promises) };
    preloadThread.detach();
    //std::async(std::launch::async, AsyncLoadFrameModel, std::move(vectors), std::move(promises));
    DebugNormalMessage("Preload Models Req End");
}

KG::Resource::FakeGeometry* KG::Resource::FakeResourceContainer::LoadGeometry(const KG::Utill::HashString& id, UINT geometryIndex)
{
    auto handle = std::make_pair(id, geometryIndex);
    if ( this->geometrys.count(handle) )
    {
        return &this->geometrys.at(handle);
    }
    else
    {
        auto metaData = ResourceLoader::LoadGeometrySetFromFile("Resource/GeometrySet.xml", id);
        if ( metaData.isRawMesh )
        {
            return this->LoadRawModel(id);
        }
        else
        {
            this->LoadModel(id);
            return this->LoadGeometry(id, geometryIndex);
        }
    }
}

KG::Resource::FakeGeometry* KG::Resource::FakeResourceContainer::LoadRawModel(const KG::Utill::HashString& id)
{
    auto* model = this->LoadModel(id);
    return &this->geometrys.emplace(std::make_pair(id, 0), model->data.meshs[0]).first->second;
}

KG::Resource::FakeGeometry* KG::Resource::FakeResourceContainer::CreateGeometry(const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh)
{
    auto handle = std::make_pair(id, geometryIndex);
    if ( this->geometrys.count(handle) )
    {
        return &this->geometrys.at(handle);
        DebugErrorMessage("Try Load Already Loaded model's Geometry")
    }
    else
    {
        return &this->geometrys.emplace(std::make_pair(handle, mesh)).first->second;
    }
}

KG::Utill::AnimationSet* KG::Resource::FakeResourceContainer::LoadAnimation(const KG::Utill::HashString& id, UINT animationIndex)
{
    auto handle = std::make_pair(id, animationIndex);
    if ( this->animations.count(handle) )
    {
        return &this->animations.at(handle);
    }
    else
    {
        auto* model = this->LoadModel(id);
        return &this->animations.emplace(std::make_pair(id, animationIndex), std::move(model->data.animations[animationIndex])).first->second;
    }
}

void KG::Resource::FakeResourceContainer::ConvertNodeToObject(const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node, const MaterialMatch& materials, KG::Core::GameObject* rootObject)
{

    object->tag = KG::Utill::HashString(node->name);
    auto* renderer = KG::Renderer::KGFakeRenderer::GetInstance();

    auto* tran = object->GetComponent<KG::Component::TransformComponent>();
    tran->SetScale(node->scale);
    tran->SetRotation(node->rotation);
    tran->SetPosition(node->position);
    if ( node->meshs.size() != 0 )
    {
        auto* geo = renderer->GetNewGeomteryComponent();
        auto* mat = renderer->GetNewMaterialComponent();

        auto& materialSet = materials.GetMaterial(object->tag);
        for ( size_t i = 0; i < node->meshs.size(); i++ )
        {
            auto index = node->meshs[i];
            geo->AddGeometry(id, index, i);
        }
        for ( size_t i = 0; i < materialSet.size(); i++ )
        {
            mat->PostMaterial(materialSet[i], i);
        }
        object->AddComponent(geo);
        object->AddComponent(mat);

        if ( geo->HasBone() )
        {
            this->postModelLoadFunctions.emplace_back(
                [renderer, rootObject, object]()
                {
                    auto* avatar = renderer->GetNewBoneTransformComponent();
                    avatar->SetRootNode(rootObject);
                    object->AddComponent(avatar);
                    object->AddComponent(renderer->GetNewRenderComponent());
                }
            );
        }
        else
        {
            object->AddComponent(renderer->GetNewRenderComponent());
        }
    }
}

KG::Core::GameObject* KG::Resource::FakeResourceContainer::CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const MaterialMatch& materials)
{
    auto& frame = this->LoadModel(id)->data;
    std::stack<std::pair<KG::Core::GameObject*, KG::Utill::ModelNode*>> modelStack;
    auto* rootObject = container.CreateNewObject();
    auto* rootModelNode = frame.root;

    for ( size_t i = 0; i < frame.meshs.size(); i++ )
    {
        this->CreateGeometry(id, i, frame.meshs[i]);
    }

    this->ConvertNodeToObject(id, rootObject, rootModelNode, materials, rootObject);


    modelStack.push(std::make_pair(rootObject, rootModelNode));
    while ( !modelStack.empty() )
    {
        auto* object = modelStack.top().first;
        auto* node = modelStack.top().second;
        auto* transform = object->GetComponent<KG::Component::TransformComponent>();
        if ( node->child != nullptr && !transform->hasChild() )
        {
            auto* childObject = container.CreateNewObject();
            auto* childNode = node->child;
            this->ConvertNodeToObject(id, childObject, childNode, materials, rootObject);
            transform->AddChild(childObject->GetComponent<KG::Component::TransformComponent>());
            modelStack.push(std::make_pair(childObject, childNode));
        }
        else if ( node->sibling != nullptr && !transform->hasNextSibiling() )
        {
            auto* siblingObject = container.CreateNewObject();
            auto* siblingNode = node->sibling;
            this->ConvertNodeToObject(id, siblingObject, siblingNode, materials, rootObject);
            transform->AddSibiling(siblingObject->GetComponent<KG::Component::TransformComponent>());
            modelStack.push(std::make_pair(siblingObject, siblingNode));
            auto* siblingObj = object->GetSibling();
            auto* child = siblingObj->GetChild();
        }
        else
        {
            modelStack.pop();
        }
    }
    //auto rootPosition = rootObject->GetTransform()->GetPosition();
    auto rootScale = rootObject->GetTransform()->GetScale();
    float scaleFactor = 0.01f;
    //rootObject->GetTransform()->SetPosition( 0, 0, 0 );
    //rootObject->GetTransform()->SetScale( rootScale.x * scaleFactor, rootScale.y * scaleFactor, rootScale.z * scaleFactor );
    auto* parentRoot = container.CreateNewObject();
    parentRoot->GetTransform()->AddChild(rootObject->GetTransform());
    return parentRoot;
}

KG::Core::GameObject* KG::Resource::FakeResourceContainer::CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const MaterialMatch& materials)
{
    auto& frame = this->LoadModel(id)->data;
    std::stack<std::pair<KG::Core::GameObject*, KG::Utill::ModelNode*>> modelStack;
    auto* rootObject = scene.CreateNewTransformObject();
    auto* rootModelNode = frame.root;



    this->ConvertNodeToObject(id, rootObject, rootModelNode, materials, rootObject);


    modelStack.push(std::make_pair(rootObject, rootModelNode));
    while ( !modelStack.empty() )
    {
        auto* object = modelStack.top().first;
        auto* node = modelStack.top().second;
        auto* transform = object->GetComponent<KG::Component::TransformComponent>();
        if ( node->child != nullptr && !transform->hasChild() )
        {
            auto* childObject = scene.CreateNewTransformObject();
            auto* childNode = node->child;
            transform->AddChild(childObject->GetComponent<KG::Component::TransformComponent>());
            this->ConvertNodeToObject(id, childObject, childNode, materials, rootObject);
            modelStack.push(std::make_pair(childObject, childNode));
        }
        else if ( node->sibling != nullptr && !transform->hasNextSibiling() )
        {
            auto* siblingObject = scene.CreateNewTransformObject();
            auto* siblingNode = node->sibling;
            transform->AddSibiling(siblingObject->GetComponent<KG::Component::TransformComponent>());
            this->ConvertNodeToObject(id, siblingObject, siblingNode, materials, rootObject);
            modelStack.push(std::make_pair(siblingObject, siblingNode));
            auto* siblingObj = object->GetSibling();
            auto* child = siblingObj->GetChild();
        }
        else
        {
            modelStack.pop();
        }
    }
    //auto rootPosition = rootObject->GetTransform()->GetPosition();
    auto rootScale = rootObject->GetTransform()->GetScale();
    float scaleFactor = 0.01f;
    //rootObject->GetTransform()->SetPosition( 0, 0, 0 );
    //rootObject->GetTransform()->SetScale( rootScale.x * scaleFactor, rootScale.y * scaleFactor, rootScale.z * scaleFactor );
    auto* parentRoot = scene.CreateNewTransformObject();
    parentRoot->GetTransform()->AddChild(rootObject->GetTransform());
    for ( auto& i : this->postModelLoadFunctions )
    {
        i();
    }
    this->postModelLoadFunctions.clear();
    return parentRoot;
}

void KG::Resource::FakeResourceContainer::Clear()
{
    this->shaders.clear();
    this->geometrys.clear();
    DebugNormalMessage(TOSTRING(FakeResourceContainer) << "All Resource Cleared");
}

KG::Resource::FakeResourceContainer* KG::Resource::FakeResourceContainer::GetInstance()
{
    //static std::once_flag onceFlags;
    //std::call_once( onceFlags, []() {FakeResourceContainer::instance = std::make_unique<FakeResourceContainer>(); } );
    if ( FakeResourceContainer::instance.get() == nullptr )
    {
        FakeResourceContainer::instance = std::make_unique<FakeResourceContainer>();
    }
    return FakeResourceContainer::instance.get();
}