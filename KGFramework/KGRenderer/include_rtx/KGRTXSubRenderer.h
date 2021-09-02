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

        KG::Resource::DXResource renderTarget;
    public:
        void Initialize(Setting setting, DXInterface dxInterface);
        void CompileShader();
        void SetGeometry();
        void CreateRootSignature();
        void CreateStateObject();
        void CreateShaderTables();
        void Render();
        auto& GetRenderTarget()
        {
            return this->renderTarget;
        }
 /*       void DiffuseRayRender();
        void ReflectionRayRender();
        void TransparentRayRender();
        void AORayRender();
        void ShadowRayRender();
        void TemporalCompileShader();*/
        //컴파일러 파고
        // 지오메트리 설정하고
        // 렌더링 연결

        // 확장은 나중에
        // 어차피 인터페이스부터 전부 다 뜯어 고쳐야 하니
        // 나중에 합시다~~~~
    };
}