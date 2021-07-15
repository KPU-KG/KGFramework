#pragma once
#include "hash.h"
#include "RenderTexture.h"
#include "ResourceMetaData.h"
#include "KGShader.h"
#include "DynamicConstantBufferManager.h"
#include <set>

namespace KG::Renderer
{
    //일단 메테리얼 관련 안함
    class PostProcess
    {
    private:
        void CreateMaterialBuffer(const KG::Resource::Metadata::ShaderSetData& data);
        ID3D10Blob* CompileShaderFromMetadata();
        ID3D12PipelineState* CreatePSO();
        void Set(ID3D12GraphicsCommandList* pd3dCommandList);
        ID3D12PipelineState* pso;
        KG::Resource::Metadata::ShaderSetData shaderSetData;
        std::map<KG::Utill::HashString, size_t> materialIndex;
        std::unique_ptr<Resource::DynamicConstantBufferManager> materialBuffer;
    public:
        int priority = 0;
        UINT unitSizeX = 1;
        UINT unitSizeY = 1;
        UINT unitSizeZ = 1;
        KG::Utill::HashString id;
        PostProcess(const KG::Resource::Metadata::ShaderSetData& data);
        ~PostProcess();
        void Draw(ID3D12GraphicsCommandList* cmdList, UINT textureSizeX, UINT textureSizeY, UINT textureSizeZ = 1 );
        size_t GetMaterialIndex(const KG::Utill::HashString& ID);
        bool CheckMaterialLoaded(const KG::Utill::HashString& ID);
        size_t RequestMaterialIndex(const KG::Utill::HashString& ID);
        Resource::DynamicElementInterface GetMaterialElement(const KG::Utill::HashString& ID);
    };

    class PostProcessor
    {
        static constexpr UINT outputCount = 2;
        UINT currentOutputIndex = 0;
        UINT width = 0;
        UINT height = 0;
        static constexpr UINT unitSize = 256;
        std::array<KG::Resource::DXResource, outputCount> outputResources;

        using ProcessPredTy = std::function<bool(const PostProcess const*, const PostProcess const*)>;
        std::multiset<PostProcess*, ProcessPredTy> processQueue;
        void Initialize();
    public:
        PostProcessor();
        void AddPostProcess(const KG::Utill::HashString& id, int priority = 0);
        void Draw(ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture, size_t cubeIndex);
        void CopyToOutput(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex);
        void CopyToResult(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex);
        void OnDrawGUI();
    };
}