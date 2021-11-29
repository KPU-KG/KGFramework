#pragma once
#include "LightComponent.h"
#include "DescriptorHeapManager.h"
#include "KGDX12Resource.h"
#include "D3D12Helper.h"
namespace KG::Renderer
{
    struct LightDataWrap
    {
        KG::Component::LightType Type;
        UINT pad[3];
        KG::Component::LightData data;
    };

    struct AmbientLightData
    {
        UINT skyboxID;
        UINT ibllut;
        UINT iblRad;
        UINT iblIrrad;
        UINT specularOutput;
    };

    class LightTable
    {
        ID3D12Resource* lightTable = nullptr;
        LightDataWrap* mappedLightTable = nullptr;

        ID3D12Resource* AmbientTable = nullptr;
        AmbientLightData* mappedAmbientTable = nullptr;

        //std::vector<LightDataWrap> lights;
        std::vector<Resource::DXResource> shadowMaps;
        Resource::DXResource specularOutput;
        UINT maxCount = 0;
    public:
        void Initialize(ID3D12Device* device, UINT maxCount, DescriptorHeapManager* heap, UINT width, UINT height);
        void CopyLight(UINT index, KG::Component::LightType lightType, KG::Component::LightData data);
        void UpdateAmbient();

        D3D12_GPU_VIRTUAL_ADDRESS GetLightGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetAmbientGPUAddress() const;
    };

};