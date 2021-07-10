#include "pch.h"
#include "PostProcess.h"
#include "RootParameterIndex.h"
#include "KGDXRenderer.h"
#include "D3D12Helper.h"
#include "DescriptorHeapManager.h"
#include "ResourceContainer.h"
#include "ImguiHelper.h"

void KG::Renderer::PostProcess::CreateMaterialBuffer(const KG::Resource::Metadata::ShaderSetData& data)
{
    auto device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
    auto elementSize = data.materialParameterSize;
    auto elementCount = 50;
    if (elementSize > 0)
    {
        DebugNormalMessage("Create Material Buffer");
        materialBuffer = std::make_unique<KG::Resource::DynamicConstantBufferManager>(device, elementSize, elementCount);
    }
}

ID3D10Blob* KG::Renderer::PostProcess::CompileShaderFromMetadata()
{
    UINT nCompileFlags = 0;
#if defined(_DEBUG)
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG_NAME_FOR_SOURCE;
#endif

    ID3DBlob* errorblob;
    ID3DBlob* shaderBlob;


    ID3DBlob* errorPreProcBlob;
    ID3DBlob* shaderPreProcBlob;

    std::wstring newFileDir;
    newFileDir.assign(this->shaderSetData.fileDir.begin(), this->shaderSetData.fileDir.end());

    HRESULT hr = ::D3DCompileFromFile(newFileDir.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        ConvertToEntryString(ShaderTarget::CS_5_1),
        ConvertToShaderString(ShaderTarget::CS_5_1),
        nCompileFlags | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
        0,
        &shaderBlob,
        &errorblob);

    if (errorblob != nullptr)
    {
        DebugErrorMessage(L"셰이더 컴파일 오류 : " << (char*)errorblob->GetBufferPointer());
    }
    return shaderBlob;
}

ID3D12PipelineState* KG::Renderer::PostProcess::CreatePSO()
{
    D3D12_CACHED_PIPELINE_STATE d3dCachedPipelineState = { };
    D3D12_COMPUTE_PIPELINE_STATE_DESC psDesc;
    ::ZeroMemory(&psDesc, sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
    
    psDesc.pRootSignature = KGDXRenderer::GetInstance()->GetPostProcessRootSignature();
    ID3DBlob* computeShader = this->CompileShaderFromMetadata();

    D3D12_SHADER_BYTECODE byteCode;
    byteCode.pShaderBytecode = computeShader->GetBufferPointer();
    byteCode.BytecodeLength = computeShader->GetBufferSize();

    psDesc.CS = byteCode;
    psDesc.NodeMask = 0;
    psDesc.CachedPSO = d3dCachedPipelineState;
    psDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    HRESULT hResult = KGDXRenderer::GetInstance()->GetD3DDevice()->CreateComputePipelineState(&psDesc, IID_PPV_ARGS(&this->pso));
    ThrowIfFailed(hResult);

    TryRelease(computeShader);

    return this->pso;
}

void KG::Renderer::PostProcess::Set(ID3D12GraphicsCommandList* pd3dCommandList)
{
    pd3dCommandList->SetPipelineState(this->pso);
    if (this->materialBuffer)
    {
        pd3dCommandList->SetGraphicsRootShaderResourceView(GraphicRootParameterIndex::MaterialData, this->materialBuffer->GetBuffer()->GetGPUVirtualAddress());
    }
}

KG::Renderer::PostProcess::PostProcess(const KG::Resource::Metadata::ShaderSetData& data)
{
    this->shaderSetData = data;
    this->unitSizeX = data.unitSizeX;
    this->unitSizeY = data.unitSizeY;
    this->unitSizeZ = data.unitSizeZ;
    this->CreatePSO();
    this->CreateMaterialBuffer(this->shaderSetData);
}

KG::Renderer::PostProcess::~PostProcess()
{
    TryRelease(this->pso);
}

void KG::Renderer::PostProcess::Draw(ID3D12GraphicsCommandList* cmdList, UINT textureSizeX, UINT textureSizeY, UINT textureSizeZ)
{
    this->Set(cmdList);
    UINT threadCountX = UINT(std::ceil(float(textureSizeX) / float(unitSizeX)));
    UINT threadCountY = UINT(std::ceil(float(textureSizeY) / float(unitSizeY)));
    UINT threadCountZ = UINT(std::ceil(float(textureSizeZ) / float(unitSizeZ)));
    cmdList->Dispatch(threadCountX, threadCountY, threadCountZ);
}

size_t KG::Renderer::PostProcess::GetMaterialIndex(const KG::Utill::HashString& ID)
{
    return this->materialIndex.at(ID);
}

bool KG::Renderer::PostProcess::CheckMaterialLoaded(const KG::Utill::HashString& ID)
{
    return this->materialIndex.count(ID);
}

size_t KG::Renderer::PostProcess::RequestMaterialIndex(const KG::Utill::HashString& ID)
{
    auto index = this->materialBuffer->RequestEmptyIndex();
    this->materialIndex.emplace(ID, index);
    return index;
}

KG::Resource::DynamicElementInterface KG::Renderer::PostProcess::GetMaterialElement(const KG::Utill::HashString& ID)
{
    auto index = this->GetMaterialIndex(ID);
    return this->materialBuffer->GetElement(index);
}

KG::Renderer::PostProcessor::PostProcessor()
    : processQueue([](const PostProcess const* a, const PostProcess const* b)-> bool { return a->priority < b->priority; })
{
    this->Initialize();
}

void KG::Renderer::PostProcessor::AddPostProcess(const KG::Utill::HashString& id, int priority)
{
    auto* process = KG::Resource::ResourceContainer::GetInstance()->LoadPostProcess(id);
    process->id = id;
    this->processQueue.insert(process);
}

void KG::Renderer::PostProcessor::Draw(ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture, size_t cubeIndex)
{
    if (this->processQueue.empty())
        return;

    auto* rootSignature = KGDXRenderer::GetInstance()->GetPostProcessRootSignature();
    auto* descHeap = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
    cmdList->SetComputeRootSignature(rootSignature);

    auto sourceHandle = descHeap->GetGPUHandle(renderTexture.renderTargetSRVIndex);
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Source, sourceHandle);

    auto gbufferHandle = descHeap->GetGPUHandle(renderTexture.gbufferSRVIndex);
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::GBufferStart, gbufferHandle);

    for (auto* process : this->processQueue)
    {
        auto prev = this->currentOutputIndex == 0 ? 1 : 0;
        cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Result, this->outputGPUHandles[currentOutputIndex]);
        cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::PrevResult, this->outputGPUHandles[prev]);

        process->Draw(cmdList, this->width, this->height, 1);
        this->currentOutputIndex = prev;
    }
    this->CopyToResult(cmdList, renderTexture.renderTarget, renderTexture, cubeIndex);
}

void KG::Renderer::PostProcessor::CopyToOutput(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* target, RenderTexture& renderTexture, size_t cubeIndex)
{
    auto barrierTargetOne = CD3DX12_RESOURCE_BARRIER::Transition(target,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    auto barrierTargetTwo = CD3DX12_RESOURCE_BARRIER::Transition(target,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    cmdList->ResourceBarrier(1, &barrierOne);
    cmdList->CopyResource(target, this->outputResources[this->currentOutputIndex]);
    cmdList->ResourceBarrier(1, &barrierTwo);
}

void KG::Renderer::PostProcessor::CopyToResult(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* target, RenderTexture& renderTexture, size_t cubeIndex)
{
    auto barrierOne = CD3DX12_RESOURCE_BARRIER::Transition(target,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COPY_DEST);
    auto barrierTwo = CD3DX12_RESOURCE_BARRIER::Transition(target,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    cmdList->ResourceBarrier(1, &barrierOne);
    cmdList->CopyResource(target, this->outputResources[this->currentOutputIndex]);
    cmdList->ResourceBarrier(1, &barrierTwo);
}


void KG::Renderer::PostProcessor::OnDrawGUI()
{
    for (auto i = this->processQueue.begin(); i != this->processQueue.end(); i++)
    {
        ImGui::Text((*i)->id.srcString.c_str());
    }
    static KG::Utill::HashString hash;
    static int priority = 0;
    ImGui::InputHashString("PostProcess ID", &hash);
    ImGui::InputInt("Priority", &priority);
    if (ImGui::SmallButton("add"))
    {
        this->AddPostProcess(hash, priority);
        hash = KG::Utill::HashString();
        priority = 0;
    }
}

void KG::Renderer::PostProcessor::Initialize()
{
    auto* renderer = KGDXRenderer::GetInstance();
    auto* device = renderer->GetD3DDevice();
    auto setting = renderer->GetSetting();
    auto* descHeap = renderer->GetDescriptorHeapManager();

    this->width = setting.clientWidth;
    this->height = setting.clientHeight;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    ZeroDesc(uavDesc);
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    for (size_t i = 0; i < this->outputCount; i++)
    {
        auto index = descHeap->RequestEmptyIndex();
        this->outputResources[i] = CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight);
        this->outputCPUHandles[i] = descHeap->GetCPUHandle(index);
        this->outputGPUHandles[i] = descHeap->GetGPUHandle(index);
        device->CreateUnorderedAccessView(this->outputResources[i], nullptr, &uavDesc, this->outputCPUHandles[i]);
    }
}
