#pragma once
#include "LightComponent.h"
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
    };

    class LightTable
    {
        ID3D12Resource* lightTable = nullptr;
        LightDataWrap* mappedLightTable = nullptr;

        ID3D12Resource* AmbientTable = nullptr;
        AmbientLightData* mappedAmbientTable = nullptr;

        //std::vector<LightDataWrap> lights;
        UINT maxCount = 0;
    public:
        void Initialize(ID3D12Device* device, UINT maxCount);
        void CopyLight(UINT index, KG::Component::LightType lightType, KG::Component::LightData data);
        void UpdateAmbient();

        D3D12_GPU_VIRTUAL_ADDRESS GetLightGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetAmbientGPUAddress() const;
    };

};