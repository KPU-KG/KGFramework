#include "pch.h"
#include "PostProcess.h"
#include "RootParameterIndex.h"
#include "KGDXRenderer.h"
#include "D3D12Helper.h"
#include "DescriptorHeapManager.h"
#include "ResourceContainer.h"
#include "ImguiHelper.h"
#include "SerializableProperty.h"

using namespace std::literals;

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
    this->commandX = "return "s + data.groupCountX;
    this->commandY = "return "s + data.groupCountY;
    this->commandZ = "return "s + data.groupCountZ;
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
    //this->unitSizeX = data.unitSizeX;
    //this->unitSizeY = data.unitSizeY;
    //this->unitSizeZ = data.unitSizeZ;
    this->CreatePSO();
    this->CreateMaterialBuffer(this->shaderSetData);
}

KG::Renderer::PostProcess::~PostProcess()
{
    TryRelease(this->pso);
}

void KG::Renderer::PostProcess::Draw(ID3D12GraphicsCommandList* cmdList, UINT cachedScreenX, UINT cachedScreenY, GroupCountParser& parser)
{
    this->Set(cmdList);
    this->GetGroupSize(cachedScreenX, cachedScreenY, parser);
    cmdList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void KG::Renderer::PostProcess::GetGroupSize(UINT cachedScreenX, UINT cachedScreenY, GroupCountParser& parser)
{
    if (cachedScreenX == this->cachedScreenX && cachedScreenY == this->cachedScreenY) return;
    this->cachedScreenX = cachedScreenX;
    this->cachedScreenY = cachedScreenY;
    this->groupCountX = parser.GetResult(this->commandX, cachedScreenX, cachedScreenY);
    this->groupCountY = parser.GetResult(this->commandY, cachedScreenX, cachedScreenY);
    this->groupCountZ = parser.GetResult(this->commandZ, cachedScreenX, cachedScreenY);
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

bool KG::Renderer::PostProcess::OnDrawGUI()
{
    bool ret = false;
    ImGui::BulletText(id.srcString.c_str());
    ImGui::SameLine();
    ImGui::Checkbox("Active", &this->isActive);
    ImGui::SameLine();
    if (ImGui::SmallButton("+"))
    {
        ret = true;
        this->priority++;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("-"))
    {
        ret = true;
        this->priority--;
    }
    return ret;
}

void KG::Renderer::PostProcess::OnDataSave(tinyxml2::XMLElement* document)
{
    auto* element = document->InsertNewChildElement("PostProcess");
    KG::Utill::XMLConverter::XMLElementSave<KG::Utill::HashString>(element, "id", this->id);
    KG::Utill::XMLConverter::XMLElementSave<int>(element, "priority", this->priority);
}

KG::Renderer::PostProcessor::PostProcessor()
    : processQueue([](const PostProcess const* a, const PostProcess const* b)-> bool { return a->priority < b->priority; })
{
    this->Initialize();
    this->copyProcess = KG::Resource::ResourceContainer::GetInstance()->LoadPostProcess("PostCopy"_id);
    this->copyProcess->id = KG::Utill::HashString("PostCopy");

    this->ssaoProcessOne = KG::Resource::ResourceContainer::GetInstance()->LoadPostProcess("PostSSAO_One"_id);
    this->ssaoProcessOne->id = KG::Utill::HashString("PostSSAO_One");

    this->ssaoProcessTwo= KG::Resource::ResourceContainer::GetInstance()->LoadPostProcess("PostSSAO_Two"_id);
    this->ssaoProcessTwo->id = KG::Utill::HashString("PostSSAO_Two");
}

void KG::Renderer::PostProcessor::AddPostProcess(const KG::Utill::HashString& id, int priority)
{
    auto* process = KG::Resource::ResourceContainer::GetInstance()->LoadPostProcess(id);
    process->id = id;
    this->processQueue.insert(process);
}

void KG::Renderer::PostProcessor::Draw(ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture, size_t cubeIndex, ID3D12Resource* cameraData)
{
    auto* rootSignature = KGDXRenderer::GetInstance()->GetPostProcessRootSignature();
    auto* descHeap = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
    cmdList->SetComputeRootSignature(rootSignature);

    if (this->processQueue.empty())
        return;

    //renderTexture.renderTargetResource.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto sourceHandle = renderTexture.renderTargetResource.GetDescriptor(KG::Resource::DescriptorType::SRV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Source, sourceHandle);

    cmdList->SetComputeRootConstantBufferView(ComputeRootParameterIndex::CameraData, cameraData->GetGPUVirtualAddress());

    auto gbufferHandle = renderTexture.gbufferTextureResources[0].GetDescriptor(KG::Resource::DescriptorType::SRV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::GBufferStart, gbufferHandle);

    auto uavbufferHandle = buffer0.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::UAVBuffers, uavbufferHandle);

    this->CopyToOutput(cmdList, renderTexture.renderTargetResource, renderTexture, cubeIndex);

    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->bufferLDR.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
    for (auto* process : this->processQueue)
    {
        if (!process->isActive) continue;
        auto prev = this->currentOutputIndex == 0 ? 1 : 0;
        cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Result,
            this->outputResources[currentOutputIndex].GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle()
        );
        cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::PrevResult,
            this->outputResources[prev].GetDescriptor(KG::Resource::DescriptorType::SRV).GetGPUHandle());

        cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::UAVBuffers,
            buffer0.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle());

        this->outputResources[currentOutputIndex].AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        this->outputResources[prev].AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        KG::Resource::DXResource::ApplyBarrierQueue(cmdList);

        process->Draw(cmdList, this->width, this->height, parser);
        if (this->debugProcess == process) this->CopyToDebug(cmdList);

        this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
        this->currentOutputIndex = prev;
    }
    this->CopyToResult(cmdList, renderTexture.renderTargetResource, renderTexture, cubeIndex);
}

void KG::Renderer::PostProcessor::SSAO(ID3D12GraphicsCommandList* cmdList, RenderTexture& renderTexture, size_t cubeIndex, ID3D12Resource* cameraData)
{
    auto* rootSignature = KGDXRenderer::GetInstance()->GetPostProcessRootSignature();
    auto* descHeap = KGDXRenderer::GetInstance()->GetDescriptorHeapManager();
    cmdList->SetComputeRootSignature(rootSignature);

    //renderTexture.renderTargetResource.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    auto sourceHandle = renderTexture.renderTargetResource.GetDescriptor(KG::Resource::DescriptorType::SRV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Source, sourceHandle);

    cmdList->SetComputeRootConstantBufferView(ComputeRootParameterIndex::CameraData, cameraData->GetGPUVirtualAddress());

    auto gbufferHandle = renderTexture.gbufferTextureResources[0].GetDescriptor(KG::Resource::DescriptorType::SRV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::GBufferStart, gbufferHandle);

    auto uavbufferHandle = buffer0.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle();
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::UAVBuffers, uavbufferHandle);

    //this->CopyToOutput(cmdList, renderTexture.renderTargetResource, renderTexture, cubeIndex);

    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->bufferLDR.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);

    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Result,
        this->bufferSSAO.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle()
    );

    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::UAVBuffers,
        buffer0.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle());

    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);

    this->ssaoProcessOne->Draw(cmdList, this->width, this->height, parser);
    if (this->debugProcess == this->ssaoProcessOne) this->CopyToDebug(cmdList);

    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);


    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);

    this->ssaoProcessTwo->Draw(cmdList, this->width, this->height, parser);
    if (this->debugProcess == this->ssaoProcessTwo) this->CopyToDebug(cmdList);

    this->buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    this->bufferSSAO.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    renderTexture.gbufferTextureResources[1].AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);

    cmdList->CopyResource(renderTexture.gbufferTextureResources[1], this->bufferSSAO);

    this->bufferSSAO.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    renderTexture.gbufferTextureResources[1].AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
}

void KG::Renderer::PostProcessor::CopyToSwapchain(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, KG::Resource::DXResource& swapchain)
{
    target.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    bufferLDR.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    ApplyBarrierQueue(cmdList);
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::Result, this->bufferLDR.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle());
    cmdList->SetComputeRootDescriptorTable(ComputeRootParameterIndex::PrevResult, target.GetDescriptor(KG::Resource::DescriptorType::UAV).GetGPUHandle());
    this->copyProcess->Draw(cmdList, this->width, this->height, parser);
    bufferLDR.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    swapchain.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    ApplyBarrierQueue(cmdList);
    cmdList->CopyResource(swapchain, bufferLDR);
}

void KG::Renderer::PostProcessor::CopyToOutput(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex)
{
    auto prev = this->currentOutputIndex == 0 ? 1 : 0;
    auto& output0 = this->outputResources[prev];
    auto& output1 = this->outputResources[this->currentOutputIndex];
    target.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    output0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    output1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
    cmdList->CopyResource(output0, target);
    cmdList->CopyResource(output1, target);
}

void KG::Renderer::PostProcessor::CopyToResult(ID3D12GraphicsCommandList* cmdList, KG::Resource::DXResource& target, RenderTexture& renderTexture, size_t cubeIndex)
{
    auto prev = this->currentOutputIndex == 0 ? 1 : 0;
    auto& output = this->outputResources[prev];
    output.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    target.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
    cmdList->CopyResource(target, output);
}

void KG::Renderer::PostProcessor::CopyToDebug(ID3D12GraphicsCommandList* cmdList)
{
    buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_SOURCE);
    buffer0Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    buffer1Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    buffer2Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COPY_DEST);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
    cmdList->CopyResource(buffer0Debug, buffer0);
    cmdList->CopyResource(buffer1Debug, buffer1);
    cmdList->CopyResource(buffer2Debug, buffer2);
    buffer0.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    buffer1.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    buffer2.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    buffer0Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    buffer1Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    buffer2Debug.AddTransitionQueue(D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    KG::Resource::DXResource::ApplyBarrierQueue(cmdList);
}


void KG::Renderer::PostProcessor::OnDrawGUI()
{
    bool sort = false;
    static bool isOpenDebug = false;
    std::array<PostProcess*, 2> ssaoProcess = {this->ssaoProcessOne, this->ssaoProcessTwo};
    if(ImGui::TreeNode("SSAO"))
    {
        for (auto i = ssaoProcess.begin(); i != ssaoProcess.end(); i++)
        {
            ImGui::PushID((*i)->id);
            sort = (*i)->OnDrawGUI();
            ImGui::SameLine();
            if (ImGui::SmallButton("Debug"))
            {
                this->debugProcess = (*i);
                isOpenDebug = true;
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("PostProcess"))
    {
        for (auto i = this->processQueue.begin(); i != this->processQueue.end(); i++)
        {
            ImGui::PushID((*i)->id);
            sort = (*i)->OnDrawGUI();
            ImGui::SameLine();
            if (ImGui::SmallButton("Debug"))
            {
                this->debugProcess = (*i);
                isOpenDebug = true;
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
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
    if (isOpenDebug)
    {
        ImGui::SetNextWindowBgAlpha(0.8f);
        if (ImGui::Begin("PostProcessDebug"))
        {
            ImGui::Text(this->debugProcess->id.srcString.c_str());

            float width = ImGui::GetWindowWidth() / 3 - 20;
            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(width - 30);
            ImGui::BulletText("Buffer 0");
            auto handle = this->buffer0Debug.GetDescriptor(DescriptorType::SRV).GetGPUHandle();
            ImGui::TextureView((ImTextureID)handle.ptr, ImVec2(width, width), "Buffer0");
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(width - 30);
            ImGui::BulletText("Buffer 1");
            handle = this->buffer1Debug.GetDescriptor(DescriptorType::SRV).GetGPUHandle();
            ImGui::TextureView((ImTextureID)handle.ptr, ImVec2(width, width), "Buffer1");
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::SetNextItemWidth(width - 30);
            ImGui::BulletText("Buffer 2");
            handle = this->buffer2Debug.GetDescriptor(DescriptorType::SRV).GetGPUHandle();
            ImGui::TextureView((ImTextureID)handle.ptr, ImVec2(width, width), "Buffer2");
            ImGui::EndGroup();
        }
        ImGui::End();
    }
}

void KG::Renderer::PostProcessor::OnDataSave(tinyxml2::XMLElement* element)
{
    for (auto& i : this->processQueue)
    {
        i->OnDataSave(element);
    }
}

void KG::Renderer::PostProcessor::OnDataLoad(tinyxml2::XMLElement* element)
{
    auto* csr = element->FirstChildElement("PostProcess");
    while (csr != nullptr)
    {
        auto id = KG::Utill::XMLConverter::XMLElementLoad<KG::Utill::HashString>(csr, "id");
        auto priority = KG::Utill::XMLConverter::XMLElementLoad<int>(csr, "priority");
        this->AddPostProcess(id, priority);
        csr = csr->NextSiblingElement("PostProcess");
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
    this->parser.Initialize();

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    ZeroDesc(uavDesc);
    uavDesc.Texture2D.MipSlice = 0;
    uavDesc.Texture2D.PlaneSlice = 0;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
    //uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    ZeroDesc(srvDesc);
    srvDesc.Texture2D.MipLevels = 0;
    srvDesc.Texture2D.PlaneSlice = 0;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


    for (size_t i = 0; i < this->outputCount; i++)
    {
        auto index = descHeap->RequestEmptyIndex();
        this->outputResources[i].SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
        this->outputResources[i].AddOnDescriptorHeap(descHeap, uavDesc);
        this->outputResources[i].AddOnDescriptorHeap(descHeap, srvDesc);
    }
    this->buffer0.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer0.AddOnDescriptorHeap(descHeap, uavDesc);
    this->buffer1.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer1.AddOnDescriptorHeap(descHeap, uavDesc);
    this->buffer2.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer2.AddOnDescriptorHeap(descHeap, uavDesc);
    this->buffer2.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer2.AddOnDescriptorHeap(descHeap, uavDesc);

    this->buffer0Debug.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer0Debug.AddOnDescriptorHeap(descHeap, srvDesc);
    this->buffer1Debug.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer1Debug.AddOnDescriptorHeap(descHeap, srvDesc);
    this->buffer2Debug.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer2Debug.AddOnDescriptorHeap(descHeap, srvDesc);
    this->buffer2Debug.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight));
    this->buffer2Debug.AddOnDescriptorHeap(descHeap, srvDesc);


    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    this->bufferLDR.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM));
    this->bufferLDR.AddOnDescriptorHeap(descHeap, uavDesc);
    this->bufferSSAO.SetResource(CreateUAVBufferResource(device, setting.clientWidth, setting.clientHeight, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM));
    this->bufferSSAO.AddOnDescriptorHeap(descHeap, uavDesc);
}
