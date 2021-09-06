#include "KGRTXSubRenderer.h"
#include "D3D12Helper.h"


#include <type_traits>
#include <atlbase.h>
#include "../lib/inc/dxcapi.h"
#include "../lib/inc/dxcapi.use.h"
#include "../lib/inc/d3d12shader.h"
#include "pix3.h"
#include "pix3_win.h"
#include "PIXEvents.h"
#include "PIXEventsCommon.h"


template <class Ty>
struct _UniqueCOMPtr
{
    using rawType = std::remove_all_extents_t<Ty>;
    rawType* ptr = nullptr;

    _UniqueCOMPtr(rawType* ptr) : ptr(ptr) {};
    ~_UniqueCOMPtr() { TryRelease(ptr); }

    operator rawType* () { return ptr; }
    rawType* operator->() { return ptr; }
    rawType** operator& () { return &ptr; }
};

static struct HLSLCompiler
{
    dxc::DxcDllSupport dxcHelper;
    _UniqueCOMPtr<IDxcLibrary> lib{ nullptr };
    _UniqueCOMPtr<IDxcCompiler> compiler{ nullptr };
    _UniqueCOMPtr<IDxcIncludeHandler> includeHandler{ nullptr };

    void Initialize()
    {
        dxcHelper.Initialize();
        dxcHelper.CreateInstance(CLSID_DxcCompiler, &compiler.ptr);
        dxcHelper.CreateInstance(CLSID_DxcLibrary, &lib.ptr);

        HRESULT hr = lib->CreateIncludeHandler(&includeHandler.ptr);
        if (FAILED(hr)) DebugErrorMessage(L"DXC Include Handler Error");
    }

    IDxcBlob* CompileShader(const std::wstring& fileName)
    {
        IDxcBlob* blob = nullptr;
        UINT codePage(0);
        _UniqueCOMPtr<IDxcBlobEncoding> shaderText = nullptr;
        _UniqueCOMPtr<IDxcBlobEncoding> error = nullptr;
        _UniqueCOMPtr<IDxcOperationResult> result = nullptr;

        lib->CreateBlobFromFile(fileName.c_str(), &codePage, &shaderText);

        compiler->Compile(shaderText, fileName.c_str(), L"main", L"lib_6_3", nullptr, 0, nullptr, 0, includeHandler, &result);
        result->GetErrorBuffer(&error);
        if (error != nullptr && error->GetBufferSize() != 0)
        {
            DebugErrorMessage(L"DXC ERROR : " << (LPCSTR)error->GetBufferPointer());
        }
        result->GetResult(&blob);
        return blob;
    }
};

static HLSLCompiler hlslCompiler;

static struct ShaderTable
{
    //256 정렬
    struct __declspec(align(256)) ShaderParam
    {
        char shaderIdentifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
        UINT64 rootParams[1];
    };

    _UniqueCOMPtr<ID3D12Resource> table = nullptr;
    ShaderParam* mappedTable = nullptr;

    void CreateBuffer(ID3D12Device5* device, UINT size)
    {
        this->table.ptr = KG::Renderer::CreateUploadHeapBuffer(device, sizeof(ShaderParam) * size);
        this->table->Map(0, nullptr, (void**)&mappedTable);
    }

    void BufferCopy(UINT index, ShaderParam& param)
    {
        this->mappedTable[index] = param;
    }

    void BufferCopy(UINT index, void* param)
    {
        memcpy(this->mappedTable + index, param, sizeof(char) * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    }
};

void KG::Renderer::RTX::KGRTXSubRenderer::Initialize(KG::Renderer::RTX::Setting setting, DXInterface dxInterface)
{
    this->setting = setting;
    this->dxInterface = dxInterface;

    dxInterface.commandList->QueryInterface(IID_PPV_ARGS(&this->rtxCommandList));
    dxInterface.device->QueryInterface(IID_PPV_ARGS(&this->rtxDevice));

    hlslCompiler.Initialize();

    this->renderTarget.SetResource(CreateUAVBufferResource(this->rtxDevice, this->setting.setting.clientWidth, this->setting.setting.clientHeight));
    //D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    //ZeroDesc(rtvDesc);
    //rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //rtvDesc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
    //rtvDesc.Texture2D.MipSlice = 0;
    //rtvDesc.Texture2D.PlaneSlice = 0;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroDesc(uavDesc);
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroDesc(srvDesc);
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    this->renderTarget.AddOnDescriptorHeap(this->dxInterface.heap, uavDesc);
    this->renderTarget.AddOnDescriptorHeap(this->dxInterface.heap, srvDesc);

    this->CreateRootSignature();
    this->CreateStateObject();
    this->CreateShaderTables();

}


void KG::Renderer::RTX::KGRTXSubRenderer::CompileShader()
{
}

void KG::Renderer::RTX::KGRTXSubRenderer::SetGeometry()
{
}

void KG::Renderer::RTX::KGRTXSubRenderer::CreateRootSignature()
{
    HRESULT result;
    constexpr UINT rangeCount = 1;
    constexpr UINT rootParamCount = 3;
    D3D12_DESCRIPTOR_RANGE rtvRange[rangeCount]{};
    rtvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    rtvRange[0].NumDescriptors = 1;
    rtvRange[0].BaseShaderRegister = 0;
    rtvRange[0].RegisterSpace = 0;
    rtvRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameter[rootParamCount]{};
    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameter[0].DescriptorTable.pDescriptorRanges = rtvRange;

    rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameter[1].Descriptor.RegisterSpace = 0;
    rootParameter[1].Descriptor.ShaderRegister = 0;

    rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].Descriptor.RegisterSpace = 0;
    rootParameter[2].Descriptor.ShaderRegister = 0;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = rootParamCount;
    rootSignatureDesc.pParameters = rootParameter;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_NONE;
    //rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
    // 레이트레이싱에서는 글로벌로 해야 하는 것 같음

    _UniqueCOMPtr<ID3DBlob> rootSignBlob = nullptr;
    _UniqueCOMPtr<ID3DBlob> errorBlob = nullptr;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignBlob, &errorBlob);
    if (FAILED(result)) DebugErrorMessage("RTX Root Signature Error");

    result = this->rtxDevice->CreateRootSignature(0, rootSignBlob->GetBufferPointer(), rootSignBlob->GetBufferSize(), IID_PPV_ARGS(&rtxRootSign));
}

void KG::Renderer::RTX::KGRTXSubRenderer::CreateStateObject()
{
    HRESULT result;
    auto* rayGenblob = hlslCompiler.CompileShader(L"Resource/ShaderScript/DXR/RAY_GEN.hlsl");
    auto* hitMissGenblob = hlslCompiler.CompileShader(L"Resource/ShaderScript/DXR/HitMiss.hlsl");

    D3D12_EXPORT_DESC rayGenExport;
    rayGenExport.Name = L"RayGeneration";
    rayGenExport.ExportToRename = nullptr;
    rayGenExport.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_EXPORT_DESC hitMissExport[2];
    hitMissExport[0].Name = L"Hit";
    hitMissExport[0].ExportToRename = nullptr;
    hitMissExport[0].Flags = D3D12_EXPORT_FLAG_NONE;

    hitMissExport[1].Name = L"Miss";
    hitMissExport[1].ExportToRename = nullptr;
    hitMissExport[1].Flags = D3D12_EXPORT_FLAG_NONE;


    D3D12_DXIL_LIBRARY_DESC rayGenerationLibDesc;
    rayGenerationLibDesc.DXILLibrary.pShaderBytecode = rayGenblob->GetBufferPointer();
    rayGenerationLibDesc.DXILLibrary.BytecodeLength = rayGenblob->GetBufferSize();
    rayGenerationLibDesc.NumExports = 1;
    rayGenerationLibDesc.pExports = &rayGenExport;

    D3D12_DXIL_LIBRARY_DESC hitMissLibDesc;
    hitMissLibDesc.DXILLibrary.pShaderBytecode = hitMissGenblob->GetBufferPointer();
    hitMissLibDesc.DXILLibrary.BytecodeLength = hitMissGenblob->GetBufferSize();
    hitMissLibDesc.NumExports = 2;
    hitMissLibDesc.pExports = hitMissExport;

    D3D12_STATE_SUBOBJECT rayGenShaders;
    rayGenShaders.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    rayGenShaders.pDesc = &rayGenerationLibDesc;

    D3D12_STATE_SUBOBJECT hitMissShaders;
    hitMissShaders.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    hitMissShaders.pDesc = &hitMissLibDesc;

    D3D12_HIT_GROUP_DESC hitGroup;
    hitGroup.HitGroupExport = L"HitGroup0";
    hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    hitGroup.AnyHitShaderImport = nullptr;
    hitGroup.ClosestHitShaderImport = L"Hit";
    hitGroup.IntersectionShaderImport = nullptr;

    D3D12_STATE_SUBOBJECT hitGroupObject;
    hitGroupObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    hitGroupObject.pDesc = &hitGroup;

    
    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
    shaderConfig.MaxPayloadSizeInBytes = sizeof(float[4]);
    shaderConfig.MaxAttributeSizeInBytes = sizeof(float[2]);

    D3D12_STATE_SUBOBJECT config;
    config.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    config.pDesc = &shaderConfig;

    D3D12_STATE_SUBOBJECT globalRS;
    globalRS.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalRS.pDesc = &rtxRootSign;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig;
    pipelineConfig.MaxTraceRecursionDepth = 1;

    D3D12_STATE_SUBOBJECT pipeline;
    pipeline.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipeline.pDesc = &pipelineConfig;

    D3D12_STATE_SUBOBJECT subobjects[] =
    {
        rayGenShaders,
        hitMissShaders,
        hitGroupObject,
        config,
        globalRS,
        pipeline
    };

    D3D12_STATE_OBJECT_DESC raytracingStateObject;
    raytracingStateObject.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    raytracingStateObject.NumSubobjects = _countof(subobjects);
    raytracingStateObject.pSubobjects = subobjects;

    result = this->rtxDevice->CreateStateObject(&raytracingStateObject, IID_PPV_ARGS(&this->stateObject));
    if (FAILED(result)) DebugErrorMessage("RTX State Object Failed");
    this->stateObject->QueryInterface(IID_PPV_ARGS(&this->stateObjectProp));

}

ShaderTable rayTable;
ShaderTable hitTable;
ShaderTable missTable;

void KG::Renderer::RTX::KGRTXSubRenderer::CreateShaderTables()
{
    rayTable.CreateBuffer(this->rtxDevice, 1);
    rayTable.BufferCopy(0, this->stateObjectProp->GetShaderIdentifier(L"RayGeneration"));

    hitTable.CreateBuffer(this->rtxDevice, 1);
    hitTable.BufferCopy(0, this->stateObjectProp->GetShaderIdentifier(L"HitGroup0"));

    missTable.CreateBuffer(this->rtxDevice, 1);
    missTable.BufferCopy(0, this->stateObjectProp->GetShaderIdentifier(L"Miss"));
}

void KG::Renderer::RTX::KGRTXSubRenderer::ReallocateInstanceBuffer()
{
    if (this->updateCount <= this->instanceBufferCount) return;
    if (this->instanceData)
    {
        this->instanceData->Unmap(0, nullptr);
        this->instanceData->Release();
        this->instanceData = nullptr;
        this->instances = nullptr;
    }
    instanceBufferCount = this->updateCount;
    this->instanceData = CreateUploadHeapBuffer(this->rtxDevice, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * this->instanceBufferCount);
    this->instanceData->Map(0, nullptr, (void**)&this->instances);
}

UINT KG::Renderer::RTX::KGRTXSubRenderer::GetUpdateCounts()
{
    auto result = updateCount;
    updateCount++;
    return result;
}

void KG::Renderer::RTX::KGRTXSubRenderer::UpdateInstanceData(UINT index, const D3D12_RAYTRACING_INSTANCE_DESC& desc)
{
    auto* target = std::next(this->instances, index);
    memcpy(target, &desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
}

void KG::Renderer::RTX::KGRTXSubRenderer::BuildTLAS()
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
    ZeroDesc(inputs);
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    inputs.NumDescs = this->updateCount;
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.InstanceDescs = this->instanceData->GetGPUVirtualAddress();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild;
    ZeroDesc(prebuild);

    //대충 업데이트할거면 기존 -> 새 버퍼 로 이중버퍼 쓰는 느낌
    this->rtxDevice->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);
    if (this->tlasScratchSize < prebuild.ScratchDataSizeInBytes)
    {
        TryRelease(this->tlasScratch);
        this->tlasScratchSize = prebuild.ScratchDataSizeInBytes;
        this->tlasScratch = CreateASBufferResource(this->rtxDevice, this->rtxCommandList, this->tlasScratchSize, D3D12_RESOURCE_STATE_COMMON);
    }

    if (this->tlasResultSize < prebuild.ResultDataMaxSizeInBytes)
    {
        TryRelease(this->tlasResult);
        this->tlasResultSize = prebuild.ResultDataMaxSizeInBytes;
        this->tlasResult = CreateASBufferResource(this->rtxDevice, this->rtxCommandList, this->tlasResultSize, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
    ZeroDesc(buildDesc);
    buildDesc.DestAccelerationStructureData = this->tlasResult->GetGPUVirtualAddress();
    buildDesc.ScratchAccelerationStructureData = this->tlasScratch->GetGPUVirtualAddress();
    buildDesc.Inputs = inputs;

    this->rtxCommandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

}

void KG::Renderer::RTX::KGRTXSubRenderer::Render()
{
    PIXBeginEvent(this->rtxCommandList, PIX_COLOR_INDEX(1), "Ray Trace");
    this->renderTarget.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->rtxCommandList->SetDescriptorHeaps(1, this->dxInterface.heap->GetAddressOf());
    this->rtxCommandList->SetComputeRootSignature(this->rtxRootSign);
    this->rtxCommandList->SetComputeRootDescriptorTable(0, this->renderTarget.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    this->rtxCommandList->SetComputeRootShaderResourceView(1, this->tlasResult->GetGPUVirtualAddress());
    this->rtxCommandList->SetComputeRootConstantBufferView(2, this->cameraData->GetGPUVirtualAddress());
    this->rtxCommandList->SetPipelineState1(this->stateObject);

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = rayTable.table->GetGPUVirtualAddress();
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderTable::ShaderParam);

    rayDesc.HitGroupTable.StartAddress = hitTable.table->GetGPUVirtualAddress();
    rayDesc.HitGroupTable.SizeInBytes = sizeof(ShaderTable::ShaderParam);
    rayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderTable::ShaderParam);

    rayDesc.MissShaderTable.StartAddress = missTable.table->GetGPUVirtualAddress();
    rayDesc.MissShaderTable.SizeInBytes = sizeof(ShaderTable::ShaderParam);
    rayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderTable::ShaderParam);

    rayDesc.Width = this->setting.setting.clientWidth;
    rayDesc.Height = this->setting.setting.clientHeight;
    rayDesc.Depth = 1;

    auto renderTargetBarrier = CD3DX12_RESOURCE_BARRIER::UAV(this->renderTarget);
    this->rtxCommandList->ResourceBarrier(1, &renderTargetBarrier);
    this->rtxCommandList->DispatchRays(&rayDesc);
    this->cameraData = nullptr;
    PIXEndEvent(this->rtxCommandList);
}

void KG::Renderer::RTX::KGRTXSubRenderer::SetCameraData(ID3D12Resource* cameraData)
{
    this->cameraData = cameraData;
}