#pragma once
#include "D3D12Helper.h"
#include "hash.h"
#include "KGShader.h"
#include <map>
#include <deque>
namespace KG::Renderer
{
    class StateObjectManager
    {
        std::vector<D3D12_STATE_SUBOBJECT> subobjects;
        std::deque<D3D12_HIT_GROUP_DESC> hitGroups;
        std::vector<LPCWSTR> shaderExportNames;
        std::map<DXRShader*, D3D12_HIT_GROUP_DESC*> hitGroupMap;
        std::map<DXRShader*, void*> hitShaderIdentifiers;
        std::map<DXRShader*, void*> missShaderIdentifiers;
        std::map<DXRShader*, void*> rayShaderIdentifiers;
        UINT localRootSignatureSubobjectIndex = 0;
        UINT shaderConfigSubobjectIndex = 0;
        ID3D12RootSignature* globalRootSignature = nullptr;
        ID3D12RootSignature* localRootSignature = nullptr;
        D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
        D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION exportRootSignAssociationDesc;
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION exportShaderConfigAssociationDesc;
        bool isDirty = true;
        ID3D12StateObject* stateObject = nullptr;
        ID3D12StateObjectProperties* stateProp = nullptr;
        void BuildStateObject(ID3D12Device5* device);
        void CreateLocalRootSignature(ID3D12Device5* device);
        void CreateDefaultSubObject();
    public:
        void Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSignature);
        void AddShader(DXRShader* shader);
        void Rebuild(ID3D12Device5* device);
        void DebugBuild(ID3D12Device5* device);
        void Set(ID3D12GraphicsCommandList4* cmdList);
        void* GetShaderIndetifier(const std::wstring& name);
        void* GetRayShaderIndetifier(DXRShader* shader);
        void* GetHitShaderIndetifier(DXRShader* shader);
        void* GetMissShaderIndetifier(DXRShader* shader);
    };
}