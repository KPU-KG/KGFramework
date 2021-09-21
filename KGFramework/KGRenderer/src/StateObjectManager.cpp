#include "StateObjectManager.h"

void KG::Renderer::StateObjectManager::BuildStateObject(ID3D12Device5* device)
{
}

void KG::Renderer::StateObjectManager::CreateLocalRootSignature(ID3D12Device5* device)
{
}

void KG::Renderer::StateObjectManager::Initialize()
{
}

void KG::Renderer::StateObjectManager::AddShader(DXRShader* shader)
{
    if (shader->isInitObject) return;

    //Build Shader
    auto& obj = subobjects.emplace_back();
    obj.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    obj.pDesc = &shader->libDesc;

    switch (shader->type)
    {
        case DXRShader::Type::RayGeneration:
        {
        }
        break;

        case DXRShader::Type::HitMiss:
        {
            //Create Hitgroup

        }
        break;
    }

    this->isDirty = true;
}

void KG::Renderer::StateObjectManager::Set(ID3D12Device5* device, ID3D12GraphicsCommandList4* cmdList)
{
    if (this->isDirty)
    {
        this->BuildStateObject(device);
    }
}

void* KG::Renderer::StateObjectManager::GetShaderIndetifier(const std::wstring& name)
{
    return this->stateProp->GetShaderIdentifier(name.c_str());
}

void* KG::Renderer::StateObjectManager::GetShaderIndetifier(DXRShader* shader)
{
    return this->GetShaderIndetifier(shader->hitgroupName);
}
