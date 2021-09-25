#include "StateObjectManager.h"
#include "D3D12Helper.h"

using namespace KG::Renderer;

void KG::Renderer::StateObjectManager::Initialize(ID3D12Device5* device, ID3D12RootSignature* globalRootSignature)
{
    this->globalRootSignature = globalRootSignature;
    this->subobjects.reserve(1000);
    this->CreateLocalRootSignature(device);
    this->CreateDefaultSubObject();
}

void KG::Renderer::StateObjectManager::BuildStateObject(ID3D12Device5* device)
{
    exportRootSignAssociationDesc.pSubobjectToAssociate = &subobjects[localRootSignatureSubobjectIndex];
    exportRootSignAssociationDesc.NumExports = shaderExportNames.size(); // Dirty면 이거 바꾸기
    exportRootSignAssociationDesc.pExports = shaderExportNames.data();

    exportShaderConfigAssociationDesc.pSubobjectToAssociate = &subobjects[shaderConfigSubobjectIndex];
    exportShaderConfigAssociationDesc.NumExports = shaderExportNames.size(); // Dirty면 이거 바꾸기
    exportShaderConfigAssociationDesc.pExports = shaderExportNames.data();

    D3D12_STATE_OBJECT_DESC raytracingStateObject;
    raytracingStateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    raytracingStateObject.NumSubobjects = subobjects.size();
    raytracingStateObject.pSubobjects = subobjects.data();

    TryRelease(stateObject);
    TryRelease(stateProp);

    HRESULT result = device->CreateStateObject(&raytracingStateObject, IID_PPV_ARGS(&stateObject));
    if (FAILED(result)) DebugErrorMessage("RTX State Obejct Creation Failed");
    result = stateObject->QueryInterface(IID_PPV_ARGS(&stateProp));
    if (FAILED(result)) DebugErrorMessage("RTX State Obejct Prop Creation Failed");
}

void KG::Renderer::StateObjectManager::CreateLocalRootSignature(ID3D12Device5* device)
{
    HRESULT result;
    constexpr UINT rootParamCount = 4;
    constexpr UINT localRootSignatureSpace = 4;

    D3D12_ROOT_PARAMETER rootParameter[rootParamCount]{};
    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[0].Descriptor.RegisterSpace = localRootSignatureSpace;
    rootParameter[0].Descriptor.ShaderRegister = 0;

    rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[1].Descriptor.RegisterSpace = localRootSignatureSpace;
    rootParameter[1].Descriptor.ShaderRegister = 1;

    rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[2].Descriptor.RegisterSpace = localRootSignatureSpace;
    rootParameter[2].Descriptor.ShaderRegister = 2;

    rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[3].Descriptor.RegisterSpace = localRootSignatureSpace;
    rootParameter[3].Descriptor.ShaderRegister = 3;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = rootParamCount;
    rootSignatureDesc.pParameters = rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

    _UniqueCOMPtr<ID3DBlob> rootSignBlob = nullptr;
    _UniqueCOMPtr<ID3DBlob> errorBlob = nullptr;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignBlob, &errorBlob);
    if (FAILED(result)) DebugErrorMessage("RTX Root Signature Error");

    result = device->CreateRootSignature(0, rootSignBlob->GetBufferPointer(), rootSignBlob->GetBufferSize(), IID_PPV_ARGS(&localRootSignature));
}

void KG::Renderer::StateObjectManager::CreateDefaultSubObject()
{
    // 쉐이더 컨픽
    {
        shaderConfig.MaxPayloadSizeInBytes = sizeof(float[4]);
        shaderConfig.MaxAttributeSizeInBytes = sizeof(float[2]);

        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        obj.pDesc = &shaderConfig;
        shaderConfigSubobjectIndex = 0;
    }
    // 글로벌 루트 시그니처
    {
        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
        obj.pDesc = &globalRootSignature;
    }
    // 파이프라인 컨픽
    {
        pipelineConfig.MaxTraceRecursionDepth = 1;

        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
        obj.pDesc = &pipelineConfig;
    }
    // 로컬 루트 시그니처
    {
        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        obj.pDesc = &localRootSignature;
        localRootSignatureSubobjectIndex = 3;
    }
    // 루트시그니처 어소시에이션
    {
        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        obj.pDesc = &exportRootSignAssociationDesc;
    }

    // 쉐이더 컨픽 어소시에이션
    {
        auto& obj = subobjects.emplace_back();
        obj.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        obj.pDesc = &exportShaderConfigAssociationDesc;
    }
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
            LPCWSTR lpcstr = shader->rayName.c_str();
            shaderExportNames.emplace_back(lpcstr);
            this->rayShaderIdentifiers[shader] = nullptr;
        }
        break;

        case DXRShader::Type::HitMiss:
        {
            //Create Hitgroup
            //EmplaceBack하면 주소 바뀌면서 터짐 -> 벡터에서 덱으로 변경
            auto& hitGroup = hitGroups.emplace_back();
            hitGroup.HitGroupExport = shader->hitgroupName.c_str();
            hitGroup.Type = D3D12_HIT_GROUP_TYPE::D3D12_HIT_GROUP_TYPE_TRIANGLES;
            hitGroup.AnyHitShaderImport = nullptr;
            hitGroup.ClosestHitShaderImport = shader->closestHitName.c_str();
            hitGroup.IntersectionShaderImport = nullptr;

            hitGroupMap[shader] = &hitGroup;

            auto& obj = subobjects.emplace_back();
            obj.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            obj.pDesc = &hitGroup;

            shaderExportNames.emplace_back(shader->hitgroupName.c_str());
            shaderExportNames.emplace_back(shader->missName.c_str());
            this->missShaderIdentifiers[shader] = nullptr;
            this->hitShaderIdentifiers[shader] = nullptr;
        }
        break;
    }
    shader->isInitObject = true;

    this->isDirty = true;
}

void KG::Renderer::StateObjectManager::Rebuild(ID3D12Device5* device)
{
    if (this->isDirty)
    {
        this->BuildStateObject(device);

        for (auto& [k, e] : this->rayShaderIdentifiers)
            e = this->GetShaderIndetifier(k->rayName);

        for (auto& [k, e] : this->hitShaderIdentifiers)
            e = this->GetShaderIndetifier(k->hitgroupName);

        for (auto& [k, e] : this->missShaderIdentifiers)
            e = this->GetShaderIndetifier(k->missName);
    }
}

void KG::Renderer::StateObjectManager::Set(ID3D12GraphicsCommandList4* cmdList)
{
    cmdList->SetPipelineState1(stateObject);
}

void* KG::Renderer::StateObjectManager::GetShaderIndetifier(const std::wstring& name)
{
    return this->stateProp->GetShaderIdentifier(name.c_str());
}

void* KG::Renderer::StateObjectManager::GetRayShaderIndetifier(DXRShader* shader)
{
    auto it = rayShaderIdentifiers.find(shader);
    void* result = nullptr;
    if (it == rayShaderIdentifiers.end() || it->second == nullptr)
    {
        rayShaderIdentifiers[shader] = nullptr;
    }
    else 
    {
        result = it->second;
    }
    return result;
}

void* KG::Renderer::StateObjectManager::GetHitShaderIndetifier(DXRShader* shader)
{
    auto it = hitShaderIdentifiers.find(shader);
    void* result = nullptr;
    if (it == hitShaderIdentifiers.end())
    {
        hitShaderIdentifiers[shader] = nullptr;
    }
    else
    {
        result = it->second;
    }
    return result;
}

void* KG::Renderer::StateObjectManager::GetMissShaderIndetifier(DXRShader* shader)
{
    auto it = missShaderIdentifiers.find(shader);
    void* result = nullptr;
    if (it == missShaderIdentifiers.end() || it->second == nullptr)
    {
        hitShaderIdentifiers[shader] = nullptr;
    }
    else
    {
        result = it->second;
    }
    return result;
}
