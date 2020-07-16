#pragma once
#include <d3d12.h>
#include "KGRenderer.h"
namespace KG::Renderer
{
	using Microsoft::WRL::ComPtr;
	class KGDXRenderer : public IKGRenderer
	{
	private:
		IDXGIFactory4* dxgiFactory = nullptr;
		IDXGISwapChain3* swapChain = nullptr;

		ID3D12Device* d3dDevice = nullptr;

	public:
		KGDXRenderer();

		virtual void Initialize() override;
		virtual void Render() override;
		virtual void OnChangeSettings(const RendererSetting& prev, const RendererSetting& next) override;
	};
}