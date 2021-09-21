#include "D3D12Helper.h"
#include "hash.h"
#include "KGShader.h"
#include <map>
namespace KG::Renderer
{
    class StateObjectManager
    {
        std::vector<D3D12_STATE_SUBOBJECT> subobjects;
        std::vector<D3D12_HIT_GROUP_DESC> hitGroups;
        std::map<DXRShader*, D3D12_HIT_GROUP_DESC*> hitgroupMap;

        bool isDirty = true;
        ID3D12StateObject* stateObject = nullptr;
        ID3D12StateObjectProperties* stateProp = nullptr;
        void BuildStateObject(ID3D12Device5* device);
        void CreateLocalRootSignature(ID3D12Device5* device);
    public:
        void Initialize();
        void AddShader(DXRShader* shader);
        void Set(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList);
        void* GetShaderIndetifier(const std::wstring& name);
        void* GetShaderIndetifier(DXRShader* shader);
    };
}