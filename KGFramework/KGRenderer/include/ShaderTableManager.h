#include <vector>
#include <map>
#include <string>
#include "D3D12Helper.h"

namespace KG::Renderer
{
    //256 정렬
    struct __declspec(align(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT * 8)) ShaderParameter
    {
        char shaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
        D3D12_GPU_VIRTUAL_ADDRESS vertexBuffer; //대충 버텍스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS indexBuffer; //대충 인덱스 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS objectBuffer; //대충 오브젝트 버퍼
        D3D12_GPU_VIRTUAL_ADDRESS materialBuffer; //대충 메테리얼 버퍼
    };

    struct ShaderTable
    {
        UINT maxSize = 0;
        ID3D12Resource* table = nullptr;
        ShaderParameter* mappedTable = nullptr;

        void ResizeBuffer(ID3D12Device* device, UINT size)
        {
            if (this->maxSize < size)
            {
                if (this->table) this->Release();

                this->table = KG::Renderer::CreateUploadHeapBuffer(device, sizeof(ShaderParameter) * size);
                this->table->Map(0, nullptr, (void**)&mappedTable);
                this->maxSize = size;
            }
        }

        void BufferCopy(UINT index, ShaderParameter& param)
        {
            this->mappedTable[index] = param;
        }

        void BufferCopy(UINT index, char* shaderIdentifier)
        {
            this->BufferCopy(index, shaderIdentifier, sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        }

        void BufferCopy(UINT index, void* param, UINT size)
        {
            memcpy(this->mappedTable + index, param, size);
        }
        
        void Release()
        {
            if (this->table)
            {
                if (mappedTable)
                {
                    this->table->Unmap(0, nullptr);
                    this->mappedTable = nullptr;
                }
                this->table->Release();
            }
        }
    };

    struct KGRenderJob;

    class ManagedShaderTable
    {
        ShaderTable table;
        std::vector<ShaderParameter> parameters;

        bool IsCapable()
        {
            return table.maxSize >= this->parameters.capacity();
        }

        void ApplyCapacity(ID3D12Device* device)
        {
            table.ResizeBuffer(device, this->parameters.capacity());
        }

        void CopyAllData()
        {
            table.BufferCopy(0, parameters.data(), sizeof(ShaderParameter) * parameters.size());
        }

        void CopyIndexData(UINT index)
        {
            table.BufferCopy(index, parameters[index]);
        }

        void CopyLastData()
        {
            auto index = this->parameters.size() - 1;
            CopyIndexData(index);
        }
    public:
        UINT Add(ID3D12Device* device, const ShaderParameter& parameter)
        {
            UINT index = this->parameters.size();
            this->parameters.push_back(parameter);
            if (this->IsCapable())
            {
                ApplyCapacity(device);
                CopyAllData();
            }
            else 
            {
                CopyLastData();
            }
            return index;
        }
        void Update(UINT index, const ShaderParameter& parameter)
        {
            this->parameters[index] = parameter;
            CopyIndexData(index);
        }
    };

    class ShaderTableManager
    {
        ManagedShaderTable rayGenerationST;
        ManagedShaderTable hitST;
        ManagedShaderTable missST;
        std::map<KGRenderJob*, UINT> hitMap;

    public:
        void AddRayGeneration(const std::wstring& id)
        {
        }

        void AddHit(const std::wstring& id, KGRenderJob* job)
        {
        }

        void UpdateHit(KGRenderJob* job)
        {
            auto index = this->hitMap[job];
            //hitST.Update(index, );
        }

        void AddMiss(const std::wstring& id)
        {
        }
    };
}