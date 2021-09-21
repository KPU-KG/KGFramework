#include "..\include\Context.h"

void KG::Renderer::Context::InitializeFirst(const ContextResource& res)
{
    this->res = res;
}

void KG::Renderer::Context::InitializeFrame(UINT frameIndex, bool isDXRPass)
{
    this->isDXRPass = isDXRPass;
}

void KG::Renderer::Context::Execute()
{
    ID3D12CommandList* lists[] = {
       (!this->isDXRPass) ?  this->res.commandList : this->res.dxrCommandList
    };
    this->res.commandQueue->ExecuteCommandLists(ARRAYSIZE(lists), lists);
}
