#pragma once
#include <d3d12.h>
#include <memory>
#include "MathHelper.h"
#include "ResourceMetadata.h"
#include "DynamicConstantBufferManager.h"
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

	enum ShaderType
	{
		Opaque = 0,
		Transparent = 2,
		LightPass = 1,
		PostProcess = 3
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

	class Shader
	{
	private:
		bool isWireFrame = false;
		ID3D12PipelineState* normalPso = nullptr;
		ID3D12PipelineState* wireframePso = nullptr;
		unsigned renderPriority = 0;
		ShaderType shaderType = ShaderType::Opaque;
		std::unique_ptr<Resource::DynamicConstantBufferManager> materialBuffer;

		void CreateMaterialBuffer(const KG::Resource::Metadata::ShaderSetData& data );
	public:
		Shader( const KG::Resource::Metadata::ShaderSetData& data );
		~Shader();
		void Set(ID3D12GraphicsCommandList* pd3dCommandList);
		void SetWireframe(bool wireframe);
		void CreateFromMetadata(const KG::Resource::Metadata::ShaderSetData& data);

		auto GetShaderType() const { return this->shaderType; }
		auto GetRenderPriority() const { return this->renderPriority; }

	private:
		static D3D12_RASTERIZER_DESC CreateRasterizerState(const KG::Resource::Metadata::ShaderSetData& data);
		static D3D12_BLEND_DESC CreateBlendState(const KG::Resource::Metadata::ShaderSetData& data);
		static D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(const KG::Resource::Metadata::ShaderSetData& data);
	public:
		static ID3D10Blob* CompileShaderFromMetadata(const KG::Resource::Metadata::ShaderCodeData& data);
	};
}