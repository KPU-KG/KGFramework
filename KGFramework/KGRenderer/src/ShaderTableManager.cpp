#include "KGRenderQueue.h"
#include "ShaderTableManager.h"

void KG::Renderer::ShaderTable::ResizeBuffer(ID3D12Device* device, UINT size)
{
    if (this->maxSize < size)
    {
        if (this->table) this->Release();

        this->table = KG::Renderer::CreateUploadHeapBuffer(device, sizeof(ShaderParameter) * size);
        this->table->Map(0, nullptr, (void**)&mappedTable);
        this->maxSize = size;
    }
}

void KG::Renderer::ShaderTable::BufferCopy(UINT index, ShaderParameter& param)
{
    memcpy(this->mappedTable + index, &param, sizeof(ShaderParameter));
}

void KG::Renderer::ShaderTable::BufferCopy(UINT index, char* shaderIdentifier)
{
    this->BufferCopy(index, shaderIdentifier, sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
}

void KG::Renderer::ShaderTable::BufferCopy(UINT index, void* param, UINT size)
{
    memcpy(this->mappedTable + index, param, size);
}

void KG::Renderer::ShaderTable::Release()
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

bool KG::Renderer::ManagedShaderTable::IsCapable()
{
    return table.maxSize >= this->parameters.capacity();
}

void KG::Renderer::ManagedShaderTable::ApplyCapacity(ID3D12Device* device)
{
    table.ResizeBuffer(device, this->parameters.capacity());
}

void KG::Renderer::ManagedShaderTable::CopyAllData()
{
    table.BufferCopy(0, parameters.data(), sizeof(ShaderParameter) * parameters.size());
}

void KG::Renderer::ManagedShaderTable::CopyIndexData(UINT index)
{
    table.BufferCopy(index, parameters[index]);
}

void KG::Renderer::ManagedShaderTable::CopyLastData()
{
    auto index = this->parameters.size() - 1;
    CopyIndexData(index);
}

UINT KG::Renderer::ManagedShaderTable::Add(ID3D12Device* device, const ShaderParameter& parameter)
{
    UINT index = this->parameters.size();
    this->parameters.push_back(parameter);
    if (this->IsCapable())
    {
        CopyLastData();
    }
    else
    {
        ApplyCapacity(device);
        CopyAllData();
    }
    return index;
}

void KG::Renderer::ManagedShaderTable::Update(UINT index, const ShaderParameter& parameter)
{
    //memcpy(
    //    reinterpret_cast<char*>(&this->parameters[index]) + sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
    //    reinterpret_cast<char*>(&parameters) + sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES,
    //    sizeof(ShaderParameter) - sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    memcpy(&this->parameters[index], &parameter, sizeof(ShaderParameter));
    CopyIndexData(index);
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::ManagedShaderTable::GetGPUAddress() const
{
    if (this->table.table == nullptr) return 0;
    return this->table.table->GetGPUVirtualAddress();
}

UINT KG::Renderer::ManagedShaderTable::GetCount() const
{
    return this->parameters.size();
}

void KG::Renderer::ShaderTableManager::CopyShaderIdentifier(RadianceShaderParameter& param, void* shaderIdentifier)
{
    if (shaderIdentifier)
        memcpy(param.shaderIdentifier, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
}

void KG::Renderer::ShaderTableManager::CopyShaderIdentifier(ShadowShaderParameter& param, void* shaderIdentifier)
{
    if (shaderIdentifier)
        memcpy(param.shaderIdentifier, shaderIdentifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
}

void KG::Renderer::ShaderTableManager::AddRayGeneration(ID3D12Device* device)
{
    ShaderParameter param;
    ZeroDesc(param);
    rayGenerationST.Add(device, param);
}

void KG::Renderer::ShaderTableManager::UpdateRay(void* shaderIdentifier, UINT index)
{
    ShaderParameter param;
    ZeroDesc(param);
    CopyShaderIdentifier(param.radiance, shaderIdentifier);
    rayGenerationST.Update(index, param);
}

void KG::Renderer::ShaderTableManager::UpdateHit(ID3D12Device* device, void* shaderIdentifier, KGRenderJob* job, int animationIndex)
{
    auto index = this->GetHitIndex(device, job, animationIndex);
    ShaderParameter param;
    CopyShaderIdentifier(param.radiance, shaderIdentifier);
    CopyShaderIdentifier(param.shadow, shadowHitIdentifier);
    param.radiance.vertexBuffer = job->GetVertexBufferGPUAddress(animationIndex);
    param.radiance.indexBuffer = job->GetIndexBufferGPUAddress();
    param.radiance.objectBuffer = job->GetObjectBufferGPUAddress();
    param.radiance.materialBuffer = job->GetMaterialBufferGPUAddress();
    hitST.Update(index, param);
}

void KG::Renderer::ShaderTableManager::UpdateMiss(ID3D12Device* device, void* shaderIdentifier, KGRenderJob* job, int animationIndex)
{
    auto index = this->GetMissIndex(device, job, animationIndex);
    ShaderParameter param;
    CopyShaderIdentifier(param.radiance, shaderIdentifier);
    CopyShaderIdentifier(param.shadow, shadowMissIdentifier);
    param.radiance.vertexBuffer = job->GetVertexBufferGPUAddress(animationIndex);
    param.radiance.indexBuffer = job->GetIndexBufferGPUAddress();
    param.radiance.objectBuffer = job->GetObjectBufferGPUAddress();
    param.radiance.materialBuffer = job->GetMaterialBufferGPUAddress();
    missST.Update(index, param);
}

UINT KG::Renderer::ShaderTableManager::GetHitIndex(ID3D12Device* device, KGRenderJob* job, int animtaionIndex)
{
    auto id = std::make_pair(job, animtaionIndex);
    if (this->hitMap.count(id)) return this->hitMap[id];
    ShaderParameter param;
    UINT index = hitST.Add(device, param);
    this->hitMap[id] = index;
    return index;
}

UINT KG::Renderer::ShaderTableManager::GetMissIndex(ID3D12Device* device, KGRenderJob* job, int animtaionIndex)
{
    auto id = std::make_pair(job, animtaionIndex);
    if (this->missMap.count(id)) return this->missMap[id];
    ShaderParameter param;
    UINT index = missST.Add(device, param);
    this->missMap[id] = index;
    return index;
}

void KG::Renderer::ShaderTableManager::PostShadowHit(void* shaderIdentifier)
{
    this->shadowHitIdentifier = shaderIdentifier;
}

void KG::Renderer::ShaderTableManager::PostShadowMiss(void* shaderIdentifier)
{
    this->shadowMissIdentifier = shaderIdentifier;
}

UINT KG::Renderer::ShaderTableManager::GetHitgroupIndex(KGRenderJob* job, int animationIndex) const
{
    auto id = std::make_pair(job, animationIndex);
    return hitMap.at(id);
}

UINT KG::Renderer::ShaderTableManager::GetHitCount() const
{
    return this->hitST.GetCount();
}

UINT KG::Renderer::ShaderTableManager::GetMissCount() const
{
    return 1;
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::ShaderTableManager::GetRayShaderTableGPUAddress(UINT index) const
{
    return rayGenerationST.GetGPUAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::ShaderTableManager::GetHitShaderTableGPUAddress() const
{
    return hitST.GetGPUAddress();
}

D3D12_GPU_VIRTUAL_ADDRESS KG::Renderer::ShaderTableManager::GetMissShaderTableGPUAddress() const
{
    return missST.GetGPUAddress();
}
