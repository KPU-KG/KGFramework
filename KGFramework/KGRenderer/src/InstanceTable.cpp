#include "InstanceTable.h"

void KG::Renderer::InstanceTable::ResizeBuffer(ID3D12Device* device, UINT size)
{
    if (this->maxSize < size)
    {
        if (this->table) this->Release();

        this->table = KG::Renderer::CreateUploadHeapBuffer(device, sizeof(UINT) * size);
        this->table->Map(0, nullptr, (void**)&mappedTable);
        this->maxSize = size;
    }
}

void KG::Renderer::InstanceTable::UpdateIndex(UINT index, UINT bufferIndex)
{
    this->mappedTable[index] = bufferIndex;
}
void KG::Renderer::InstanceTable::Release()
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
