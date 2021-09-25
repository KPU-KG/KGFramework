#include <vector>
#include <map>
#include <string>
#include "D3D12Helper.h"

namespace KG::Renderer
{
    //256 ����
    struct __declspec(align(256)) ShaderParameter
    {
        char shaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
        D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer; //���� ���ؽ� ����
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer; //���� �ε��� ����
        D3D12_GPU_VIRTUAL_ADDRESS objectBuffer; //���� ������Ʈ ����
        D3D12_GPU_VIRTUAL_ADDRESS materialBuffer; //���� ���׸��� ����
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
    };

    class StateObjectManager;
    class ShaderTableManager
    {
        ManagedShaderTable rayGenerationST;
        ManagedShaderTable hitST;
        ManagedShaderTable missST;
        std::map<KGRenderJob*, UINT> hitMap;

        static void CopyShaderIdentifier(ShaderParameter& param, void* shaderIdentifier);
    public:
        void AddRayGeneration(ID3D12Device* device);

        void AddHit(ID3D12Device* device, KGRenderJob* job);
        void AddMiss(ID3D12Device* device, KGRenderJob* job);

        void UpdateRay(void* shaderIdentifier);
        void UpdateHit(void* shaderIdentifier, KGRenderJob* job);
        void UpdateMiss(void* shaderIdentifier, KGRenderJob* job);

        UINT GetHitgroupIndex(KGRenderJob* job) const;

        D3D12_GPU_VIRTUAL_ADDRESS GetRayShaderTableGPUAddress(UINT index = 0) const;
        D3D12_GPU_VIRTUAL_ADDRESS GetHitShaderTableGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetMissShaderTableGPUAddress() const;
    };
}