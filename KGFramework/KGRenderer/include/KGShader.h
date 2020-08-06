#pragma once
#include <d3d12.h>
#include <memory>
#include "MathHelper.h"
namespace KG::Renderer
{
	using namespace DirectX;
	class Camera;
	enum class ShaderTarget
	{
		CS_5_0, CS_5_1,
		VS_5_0, VS_5_1,
		DS_5_0, DS_5_1,
		GS_5_0, GS_5_1,
		HS_5_0, HS_5_1,
		PS_5_0, PS_5_1,
	};

	constexpr const char* ConvertToShaderString(ShaderTarget target)
	{
		switch (target)
		{
			case ShaderTarget::CS_5_0: return "cs_5_0";
			case ShaderTarget::CS_5_1: return "cs_5_1";
			case ShaderTarget::VS_5_0: return "vs_5_0";
			case ShaderTarget::VS_5_1: return "vs_5_1";
			case ShaderTarget::DS_5_0: return "ds_5_0";
			case ShaderTarget::DS_5_1: return "ds_5_1";
			case ShaderTarget::GS_5_0: return "gs_5_0";
			case ShaderTarget::GS_5_1: return "gs_5_1";
			case ShaderTarget::HS_5_0: return "hs_5_0";
			case ShaderTarget::HS_5_1: return "hs_5_1";
			case ShaderTarget::PS_5_0: return "ps_5_0";
			case ShaderTarget::PS_5_1: return "ps_5_1";
		}
	}

	struct MaterialConstant
	{

	};

	class FutureIShader
	{
	protected:
		bool isWireFrame = false;
		ID3D12PipelineState* normalPso = nullptr;
		ID3D12PipelineState* wireframePso = nullptr;
	public:
		virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
		virtual D3D12_BLEND_DESC CreateBlendState();
		virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
		virtual void CreateShader();
		virtual void Set(ID3D12GraphicsCommandList* pd3dCommandList);
		void SetWireframe(bool wireframe);
	};

	class IShader
	{
	protected:
		bool isWireFrame = false;
		ID3D12PipelineState* normalPso = nullptr;
		ID3D12PipelineState* wireframePso = nullptr;
	public:
		using INPUT_LAYOUT_PAIR = std::pair<D3D12_INPUT_LAYOUT_DESC, std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]>>;
		virtual INPUT_LAYOUT_PAIR CreateInputLayout();
		virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
		virtual D3D12_BLEND_DESC CreateBlendState();
		virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
		virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
		virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
		virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dRootSignature);

		virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
		virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
		virtual void ReleaseShaderVariables();

		virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
		virtual void Set(ID3D12GraphicsCommandList* pd3dCommandList);
		void SetWireframe(bool wireframe);

		static D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
		static D3D12_SHADER_BYTECODE CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, ShaderTarget pszShaderProfile, ID3DBlob** ppd3dShaderBlob);
	};
}