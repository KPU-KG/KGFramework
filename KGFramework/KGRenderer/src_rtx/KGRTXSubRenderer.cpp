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
            DebugErrorMessage(L"DXC ERROR : " << error->GetBufferPointer());
        }
        result->GetResult(&blob);
        return blob;
    }
};

static HLSLCompiler hlslCompiler;

static struct ShaderTable
{
    struct ShaderParam
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
    constexpr UINT rootParamCount = 1;
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
    auto* blob = hlslCompiler.CompileShader(L"Resource/ShaderScript/DXR/RAY_GEN.hlsl");

    D3D12_EXPORT_DESC rayGenExport;
    rayGenExport.Name = L"RayGeneration";
    rayGenExport.ExportToRename = nullptr;
    rayGenExport.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_DXIL_LIBRARY_DESC rayGenerationLibDesc;
    rayGenerationLibDesc.DXILLibrary.pShaderBytecode = blob->GetBufferPointer();
    rayGenerationLibDesc.DXILLibrary.BytecodeLength = blob->GetBufferSize();
    rayGenerationLibDesc.NumExports = 1;
    rayGenerationLibDesc.pExports = &rayGenExport;

    D3D12_STATE_SUBOBJECT shaders;
    shaders.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    shaders.pDesc = &rayGenerationLibDesc;

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig;
    shaderConfig.MaxPayloadSizeInBytes = 0;
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
        shaders,
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

void KG::Renderer::RTX::KGRTXSubRenderer::CreateShaderTables()
{
    rayTable.CreateBuffer(this->rtxDevice, 1);
    rayTable.BufferCopy(0, this->stateObjectProp->GetShaderIdentifier(L"RayGeneration"));
}

void KG::Renderer::RTX::KGRTXSubRenderer::Render()
{
    PIXBeginEvent(this->rtxCommandList, PIX_COLOR_INDEX(1), "Ray Trace");
    this->renderTarget.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->rtxCommandList->SetDescriptorHeaps(1, this->dxInterface.heap->GetAddressOf());
    this->rtxCommandList->SetComputeRootSignature(this->rtxRootSign);
    this->rtxCommandList->SetComputeRootDescriptorTable(0, this->renderTarget.GetDescriptor(DescriptorType::UAV).GetGPUHandle());
    this->rtxCommandList->SetPipelineState1(this->stateObject);

    D3D12_DISPATCH_RAYS_DESC rayDesc;
    ZeroDesc(rayDesc);
    rayDesc.RayGenerationShaderRecord.StartAddress = rayTable.table->GetGPUVirtualAddress();
    rayDesc.RayGenerationShaderRecord.SizeInBytes = sizeof(ShaderTable::ShaderParam);
    rayDesc.Width = this->setting.setting.clientWidth;
    rayDesc.Height = this->setting.setting.clientHeight;
    rayDesc.Depth = 1;

    this->rtxCommandList->DispatchRays(&rayDesc);
    PIXEndEvent(this->rtxCommandList);
}

//void KG::Renderer::RTX::KGRTXSubRenderer::CompileShader()
//{
//    _UniqueCOMPtr<IDxcUtils> utills{ nullptr };
//    _UniqueCOMPtr<IDxcCompiler3> compiler{ nullptr };
//
//    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utills));
//    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
//
//    _UniqueCOMPtr<IDxcIncludeHandler> includeHandler{ nullptr };
//    utills->CreateDefaultIncludeHandler(&includeHandler);
//
//    LPCWSTR args[] =
//    {
//        L"myshader.hlsl",            // Optional shader source file name for error reporting and for PIX shader source view.  
//        L"-E", L"main",              // Entry point.
//        L"-T", L"lib_6_3",            // Target.
//        L"-Zs",                      // Enable debug information (slim format)
//        L"-D", L"MYDEFINE=1",        // A single define.
//        L"-Fo", L"myshader.bin",     // Optional. Stored in the pdb. 
//        L"-Fd", L"myshader.pdb",     // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
//        L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
//    };
//
//    _UniqueCOMPtr<IDxcBlobEncoding> source = nullptr;
//    utills->LoadFile(L"myshader.hlsl", nullptr, &source);
//    DxcBuffer buffer;
//    buffer.Ptr = source->GetBufferPointer();
//    buffer.Size = source->GetBufferSize();
//    buffer.Encoding = DXC_CP_ACP;
//
//    _UniqueCOMPtr<IDxcResult> result = nullptr;
//    compiler->Compile(&buffer, args, _countof(args), includeHandler, IID_PPV_ARGS(&result));
//
//    _UniqueCOMPtr<IDxcBlobUtf8> errors = nullptr;
//    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
//
//    if(errors != nullptr && errors->GetStringLength() != 0)
//        DebugErrorMessage()
//    //result->GetOutput(DXC_OUT_ERRORS, IID)
//
//
//
//    //DxcCreateInstance()
//}