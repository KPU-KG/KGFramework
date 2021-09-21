#include <concurrent_queue.h>
#include <array>

#include "D3D12Helper.h"
#include "KGDXRenderer.h"

namespace KG::Renderer
{
    struct Context;
    struct ContextQueue
    {
        Concurrency::concurrent_queue<Context*> queue;
        void ReserveQueue(UINT size)
        {
            for (size_t i = 0; i < size; i++)
            {
                queue.push(new Context);
            }
        }
        Context* AllocateContext()
        {
            Context* result = nullptr;
            while (1)
            {
                if (queue.try_pop(result))
                    break;
                else
                    ReserveQueue(2);
            }
                
        }
        void DeallocateContext(Context* context)
        {
            queue.push(context);
        }
    };

    struct ContextResource
    {
        ID3D12Device5* dxrDevice = nullptr;
        ID3D12Device* device = nullptr;

        ID3D12GraphicsCommandList4* dxrCommandList = nullptr;
        ID3D12GraphicsCommandList* commandList = nullptr;
        std::array<ID3D12CommandAllocator*, maxFrameCount> commandAlloc{ nullptr, nullptr, nullptr };

        ID3D12CommandQueue* commandQueue = nullptr;

        HardwareFeature hwFeature;
    };

    struct Context
    {
        UINT currentFrameIndex = -1;
        bool isDXRPass;
        ContextResource res;

        void InitializeFirst(const ContextResource& res);
        void InitializeFrame(UINT frameIndex, bool isDXRPass);
        void Execute();

#
    };
};