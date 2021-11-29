#include <vector>
#include <map>
#include <string>
#include "D3D12Helper.h"

namespace KG::Renderer
{
    struct __declspec(align(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT)) RadianceShaderParameter
    {
        char shaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
        D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer; //대충 버텍스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer; //대충 인덱스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS objectBuffer; //대충 오브젝트 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS materialBuffer; //대충 메테리얼 버퍼
    };

    struct __declspec(align(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT)) ShadowShaderParameter
    {
        char shaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
        D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer; //대충 버텍스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer; //대충 인덱스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS objectBuffer; //대충 오브젝트 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS materialBuffer; //대충 메테리얼 버퍼
    };

    //256 정렬
    struct  ShaderParameter
    {
        RadianceShaderParameter radiance;
        ShadowShaderParameter shadow;
    };


    struct ShaderTable
    {
        UINT maxSize = 0;
        ID3D12Resource* table = nullptr;
        ShaderParameter* mappedTable = nullptr;

        void ResizeBuffer(ID3D12Device* device, UINT size);

        void BufferCopy(UINT index, ShaderParameter& param);

        void BufferCopy(UINT index, char* shaderIdentifier);

        void BufferCopy(UINT index, void* param, UINT size);
        
        void Release();
    };

    struct KGRenderJob;

    class ManagedShaderTable
    {
        ShaderTable table;
        std::vector<ShaderParameter> parameters;

        bool IsCapable();

        void ApplyCapacity(ID3D12Device* device);

        void CopyAllData();

        void CopyIndexData(UINT index);

        void CopyLastData();
    public:
        UINT Add(ID3D12Device* device, const ShaderParameter& parameter);
        void Update(UINT index, const ShaderParameter& parameter);
        D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;
        UINT GetCount() const;
    };

    class StateObjectManager;
    class ShaderTableManager
    {
        ManagedShaderTable rayGenerationST;
        ManagedShaderTable hitST;
        ManagedShaderTable missST;
        std::map<std::pair<KGRenderJob*, int>, UINT> hitMap;
        std::map<std::pair<KGRenderJob*, int>, UINT> missMap;
        void* shadowHitIdentifier = nullptr;
        void* shadowMissIdentifier = nullptr;

        UINT GetHitIndex(ID3D12Device* device, KGRenderJob* job, int animtaionIndex = -1);
        UINT GetMissIndex(ID3D12Device* device, KGRenderJob* job, int animtaionIndex = -1);

        static void CopyShaderIdentifier(RadianceShaderParameter& param, void* shaderIdentifier);
        static void CopyShaderIdentifier(ShadowShaderParameter& param, void* shaderIdentifier);
    public:
        void AddRayGeneration(ID3D12Device* device);


        void PostShadowHit(void* shaderIdentifier);
        void PostShadowMiss(void* shaderIdentifier);

        void UpdateRay(void* shaderIdentifier, UINT index);
		void UpdateHit(ID3D12Device* device, void* shaderIdentifier, KGRenderJob* job, int animationIndex);
		void UpdateMiss(ID3D12Device* device, void* shaderIdentifier, KGRenderJob* job, int animationIndex);

        UINT GetHitgroupIndex(KGRenderJob* job, int animationIndex = -1) const;

        UINT GetHitCount() const;
        UINT GetMissCount() const;

        D3D12_GPU_VIRTUAL_ADDRESS GetRayShaderTableGPUAddress(UINT index = 0) const;
        D3D12_GPU_VIRTUAL_ADDRESS GetHitShaderTableGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetMissShaderTableGPUAddress() const;
    };
}