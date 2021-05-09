#pragma once
#include <map>
#include <memory>
#include <vector>
#include <future>
#include <functional>
#include "hash.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "KGModel.h"
#include "KGShader.h"
#include "Texture.h"
#include "GameObject.h"
#include "ObjectContainer.h"
#include "BoneDataBufferManager.h"
namespace KG::Resource
{
    struct FakeGeometry
    {
        bool hasBone = false;
        std::vector<KG::Utill::HashString> boneIds;
        KG::Resource::BoneData bones;

        FakeGeometry(const KG::Utill::MeshData& data)
        {
            for ( size_t i = 0; i < data.bones.size(); i++ )
            {
                this->hasBone = true;
                this->bones.offsetMatrixs[i] = Math::Matrix4x4::Transpose(data.bones[i].offsetMatrix);
                this->boneIds.push_back(data.bones[i].nodeId);
            }
        }
        auto HasBone() const
        {
            return this->hasBone;
        };
    };


    class FakeResourceContainer
    {
    private:
        std::map<KG::Utill::HashString, KG::Renderer::Shader> shaders;

        std::map<KG::Utill::HashString, std::future<KG::Resource::FrameModel>> preloadModels;
        std::map<KG::Utill::HashString, KG::Resource::FrameModel> models;


        std::map<std::pair<KG::Utill::HashString, UINT>, KG::Resource::FakeGeometry> geometrys;
        std::map<std::pair<KG::Utill::HashString, UINT>, KG::Utill::AnimationSet> animations;
        static std::unique_ptr<FakeResourceContainer> instance;
    public:
        FakeResourceContainer();
        ~FakeResourceContainer();

        FakeResourceContainer(const FakeResourceContainer& other) = delete;
        FakeResourceContainer(FakeResourceContainer&& other) = delete;
        FakeResourceContainer& operator=(const FakeResourceContainer& other) = delete;
        FakeResourceContainer& operator=(FakeResourceContainer&& other) = delete;

        KG::Resource::FrameModel* LoadModel(const KG::Utill::HashString& id);
        void PreLoadModels(std::vector<KG::Utill::HashString>&& vectors);
        KG::Resource::FakeGeometry* LoadGeometry(const KG::Utill::HashString& id, UINT geometryIndex = 0);
        KG::Resource::FakeGeometry* LoadRawModel(const KG::Utill::HashString& id);
        KG::Resource::FakeGeometry* CreateGeometry(const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh);
        KG::Utill::AnimationSet* LoadAnimation(const KG::Utill::HashString& id, UINT animationIndex);

        std::vector<std::function<void()>> postModelLoadFunctions;
        void ConvertNodeToObject(const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node, const MaterialMatch& materials, KG::Core::GameObject* rootObject);
        KG::Core::GameObject* CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const MaterialMatch& materials);
        KG::Core::GameObject* CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const MaterialMatch& materials);

        void Clear();
        static FakeResourceContainer* GetInstance();
    };
};