#include "LightTable.h"
#include "ResourceContainer.h"
#include "KGDXRenderer.h"

void KG::Renderer::LightTable::Initialize(ID3D12Device* device, UINT maxCount, DescriptorHeapManager* heap, UINT width, UINT height)
{
    this->lightTable = CreateUploadHeapBuffer(device, maxCount * sizeof(LightDataWrap));
    this->lightTable->Map(0, 0, (void**)&mappedLightTable);

    this->AmbientTable = CreateUploadHeapBuffer(device, ConstantBufferSize(sizeof(AmbientLightData) * 1));
    this->AmbientTable->Map(0, 0, (void**)&mappedAmbientTable);
    this->maxCount = maxCount;


    D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroDesc(desc);
    desc.Format = DXGI_FORMAT_R8_SNORM;
    desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    desc.Texture2D.PlaneSlice = 0;
    this->shadowMaps.resize(maxCount);
    for (size_t i = 0; i < maxCount; i++)
    {
        this->shadowMaps[i] = CreateUAVBufferResource(device, width, height, DXGI_FORMAT::DXGI_FORMAT_R8_SNORM);
        this->shadowMaps[i].AddOnDescriptorHeap(heap, desc);
    }
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    this->specularOutput = CreateUAVBufferResource(device, width, height);
    this->specularOutput.AddOnDescriptorHeap(heap, desc);
}

void KG::Renderer::LightTable::CopyLight(UINT index, KG::Component::LightType lightType, KG::Component::LightData data)
{
    LightDataWrap wrap;
    ZeroDesc(wrap);
    wrap.data = data;
    wrap.Type = lightType;
    wrap.pad[0] = this->shadowMaps[index].GetDescriptor(DescriptorType::UAV).HeapIndex;
    mappedLightTable[index] = wrap;
}

void KG::Renderer::LightTable::UpdateAmbient()
{
    auto skyboxId = KGDXRenderer::GetInstance()->GetSkymapTexutreId();
	if (skyboxId != 0)
	{
		auto RadId = skyboxId == "SkyDay"_id ? "SkyDay_radiance"_id : "SkySunset_radiance"_id;
		auto IrradId = skyboxId == "SkyDay"_id ? "SkyDay_irradiance"_id : "SkySunset_irradiance"_id;
		auto lutId = "iblLUT"_id;
		mappedAmbientTable->skyboxID = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(skyboxId)->index;
		mappedAmbientTable->ibllut = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(lutId)->index;;
		mappedAmbientTable->iblIrrad = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(IrradId)->index;;
		mappedAmbientTable->iblRad = KG::Resource::ResourceContainer::GetInstance()->LoadTexture(RadId)->index;;
		mappedAmbientTable->specularOutput = this->specularOutput.GetDescriptor(DescriptorType::UAV).HeapIndex;
	}
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::LightTable::GetLightGPUAddress() const
{
    return this->lightTable->GetGPUVirtualAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::LightTable::GetAmbientGPUAddress() const
{
    return this->AmbientTable->GetGPUVirtualAddress();
}
