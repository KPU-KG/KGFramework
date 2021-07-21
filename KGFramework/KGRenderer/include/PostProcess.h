#pragma once
#include "hash.h"
#include "RenderTexture.h"
#include "ResourceMetaData.h"
#include "KGShader.h"
#include "DynamicConstantBufferManager.h"
#include "SerializableProperty.h"
#include "KGLua.h"
#include <set>

namespace KG::Renderer
{
    struct GroupCountParser
    {
        lua_State* luaState;
        void Initialize()
        {
            luaState = KG::Lua::OpenCommonState();
        }
        int GetResult(const std::string& command, int sx, int sy)
        {
            lua_pushinteger(luaState, sx);
            lua_setglobal(luaState, "sx");
            lua_pushinteger(luaState, sy);
            lua_setglobal(luaState, "sy");
            luaL_loadstring(luaState, command.c_str());
            lua_call(luaState, 0, 1);
            int result = lua_tonumber(luaState, -1);
            lua_settop(luaState, 0);
            return result;
        }
    };


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
        bool isActive = true;
        int priority = 0;

        UINT cachedScreenX = 0;
        UINT cachedScreenY = 0;

        UINT groupCountX = 1;
        UINT groupCountY = 1;
        UINT groupCountZ = 1;

        std::string commandX;
        std::string commandY;
        std::string commandZ;

        KG::Utill::HashString id;

        PostProcess(const KG::Resource::Metadata::ShaderSetData& data);
        ~PostProcess();
        void Draw(ID3D12GraphicsCommandList* cmdList, UINT cachedScreenX, UINT cachedScreenY, GroupCountParser& parser);
        void GetGroupSize(UINT cachedScreenX, UINT cachedScreenY, GroupCountParser& parser);
        size_t GetMaterialIndex(const KG::Utill::HashString& ID);
        bool CheckMaterialLoaded(const KG::Utill::HashString& ID);
        size_t RequestMaterialIndex(const KG::Utill::HashString& ID);
        Resource::DynamicElementInterface GetMaterialElement(const KG::Utill::HashString& ID);
        bool OnDrawGUI();
        void OnDataSave(tinyxml2::XMLElement* element);
    };

    class PostProcessor
    {
        static constexpr UINT outputCount = 2;
        UINT currentOutputIndex = 0;
        UINT width = 0;
        UINT height = 0;
        static constexpr UINT unitSize = 256;
        std::array<KG::Resource::DXResource, outputCount> outputResources;

        KG::Resource::DXResource buffer0;
        KG::Resource::DXResource buffer1;
        KG::Resource::DXResource buffer2;

        KG::Resource::DXResource bufferLDR;

        PostProcess* copyProcess;
        GroupCountParser parser;


        using ProcessPredTy = std::function<bool(const PostProcess const*, const PostProcess const*)>;
        std::multiset<PostProcess*, ProcessPredTy> processQueue;
        void Initialize();
    public:
        PostProcessor();
        void AddPostProcess(const KG::Utill::HashString& id, int priority = 0);
        void Draw(ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture, size_t cubeIndex);
        void CopyToSwapchain(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, KG::Resource::DXResource& swapchain);
        void CopyToOutput(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex);
        void CopyToResult(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex);
        void OnDrawGUI();
        void OnDataSave(tinyxml2::XMLElement* element);
        void OnDataLoad(tinyxml2::XMLElement* element);
    };
}