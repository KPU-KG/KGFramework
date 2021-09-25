#include "LightTable.h"
#include "ResourceContainer.h"
#include "KGDXRenderer.h"


void KG::Renderer::LightTable::Initialize(ID3D12Device* device, UINT maxCount)
{
    this->lightTable = CreateUploadHeapBuffer(device, maxCount * sizeof(LightDataWrap));
    this->lightTable->Map(0, 0, (void**)&mappedLightTable);

    this->AmbientTable = CreateUploadHeapBuffer(device, ConstantBufferSize(sizeof(AmbientLightData) * 1));
    this->AmbientTable->Map(0, 0, (void**)&mappedAmbientTable);
    this->maxCount = maxCount;
}

void KG::Renderer::LightTable::CopyLight(UINT index, KG::Component::LightType lightType, KG::Component::LightData data)
{
    LightDataWrap wrap;
    ZeroDesc(wrap);
    wrap.data = data;
    wrap.Type = lightType;
    mappedLightTable[index] = wrap;
}

void KG::Renderer::LightTable::UpdateAmbient()
{
    auto skyboxId = KGDXRenderer::GetInstance()->GetSkymapTexutreId();
    auto RadId = skyboxId == "SkyDay"_id ? "SkyDay_radiance"_id : "SkySunset_radiance"_id;
    auto IrradId = skyboxId == "SkyDay"_id ? "SkyDay_irradiance"_id : "SkySunset_irradiance"_id;
    auto lutId = "iblLUT"_id;
    mappedAmbientTable->skyboxID = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(skyboxId)->index;
    mappedAmbientTable->ibllut = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(lutId)->index;;
    mappedAmbientTable->iblIrrad = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(IrradId)->index;;
    mappedAmbientTable->iblRad = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(RadId)->index;;
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::LightTable::GetLightGPUAddress() const
{
    return this->lightTable->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::LightTable::GetAmbientGPUAddress() const
{
    return this->AmbientTable->GetGPUVirtualAddress();
}
