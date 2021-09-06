#pragma once
#include <dxgi1_4.h>
#include <d3d12.h>
#include <vector>
#include <functional>
#include "KGDX12Resource.h"
#include "KGDXRenderer.h"
namespace KG::Renderer::RTX
{
    struct Setting
    {
        bool useDiffuseRaytrace = false;
        bool useReflectionRaytrace = false;
        bool useTransparentRaytrace = false;
        bool useAORaytrace = false;
        bool useShadowRaytrace = false;
        KG::Renderer::RendererSetting setting;
    };
    struct DXInterface
    {
        ID3D12CommandList* commandList = nullptr;
        ID3D12Device* device = nullptr;
        DescriptorHeapManager* heap = nullptr;
    };




    class KGRTXSubRenderer
    {
        KGDXRenderer* dxRenderer;
        Setting setting;
        DXInterface dxInterface;
        // D3D12 Interface
        ID3D12GraphicsCommandList4* rtxCommandList = nullptr;
        ID3D12Device5* rtxDevice = nullptr;
        ID3D12RootSignature* rtxRootSign = nullptr;
        ID3D12StateObject* stateObject = nullptr;
        ID3D12StateObjectProperties* stateObjectProp = nullptr;

        ID3D12Resource* tlasResult = nullptr;
        ID3D12Resource* tlasScratch = nullptr;
        ID3D12Resource* instanceData = nullptr;
        D3D12_RAYTRACING_INSTANCE_DESC* instances = nullptr;

        UINT tlasResultSize = 0;
        UINT tlasScratchSize = 0;
        UINT instanceBufferCount = 0;
        UINT updateCount = 0;

        KG::Resource::DXResource renderTarget;
        ID3D12Resource* cameraData = nullptr;
    public:
        void Initialize(Setting setting, DXInterface dxInterface);
        void CompileShader();
        void SetGeometry();
        void CreateRootSignature();
        void CreateStateObject();
        void CreateShaderTables();
        void ReallocateInstanceBuffer();
        UINT GetUpdateCounts();
        void UpdateInstanceData(UINT index, const D3D12_RAYTRACING_INSTANCE_DESC& desc);
        void BuildTLAS();
        void Render();
        void SetCameraData(ID3D12Resource* cameraData);
        auto& GetRenderTarget()
        {
            return this->renderTarget;
        }
        auto* GetCommandList() const
        {
            return this->rtxCommandList;
        };
        auto* GetDevice() const
        {
            return this->rtxDevice;
        };
 /*       void DiffuseRayRender();
        void ReflectionRayRender();
        void TransparentRayRender();
        void AORayRender();
        void ShadowRayRender();
        void TemporalCompileShader();*/
        //�����Ϸ� �İ�
        // ������Ʈ�� �����ϰ�
        // ������ ����

        // Ȯ���� ���߿�
        // ������ �������̽����� ���� �� ��� ���ľ� �ϴ�
        // ���߿� �սô�~~~~
    };
}