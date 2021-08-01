#pragma once
#include <d3d12.h>
#include <map>
#include <memory>
#include "hash.h"
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

	enum ShaderGroup
	{
		Opaque = 0,
		SkyBox = 1,
		Transparent = 2,
		AmbientLight = 3,
		MeshVolumeLight = 4,
		DirectionalLight = 5,
		ParticleAdd = 6,
		ParticleTransparent = 7,
        Sprite = 8,
        UI = 9,
        MaxShaderCount,
	};

	// 지오메트리 타입에 따라 결정
	enum class ShaderMeshType
	{
		StaticMesh = 0,
		SkinnedMesh = 1,
		ParticleTransparent = 2,
	};

	enum class ShaderTesselation
	{
		NormalMesh = 0,
		TesselationMesh = 1,
		LightVolumeMesh = 2,
	};

	// 최종 렌더 명령시에 결정
	enum class ShaderGeometryType
	{
		Default = 0,
		GeometryCubeMap = 1,
		Light = 2,
		SkyBox = 3,
		GSCubeShadow = 4,
		GSCascadeShadow = 5,
		Particle = 6,
	};
	// 최종 렌더 명령시에 결정
	enum class ShaderPixelType
	{
		Deferred = 0,
		GreenWireFrame = 1,
		GeometricCubeMapDeferred = 2,
		Light = 3,
		Shadow = 4,
		GSCubeShadow = 5,
		Forward = 6,
		Transparent = 7,
		SkyBox = 8,
		Add = 9,
	};

	constexpr const char* ConvertToMacroString( ShaderGroup target)
	{
		switch ( target )
		{
			case KG::Renderer::Opaque:
				return "Opaque";
			case KG::Renderer::SkyBox:
				return "SkyBox";
			case KG::Renderer::Transparent:
				return "Transparent";
			case KG::Renderer::AmbientLight:
				return "AmbientLight";
			case KG::Renderer::MeshVolumeLight:
				return "MeshVolumeLight";
			case KG::Renderer::DirectionalLight:
				return "DirectionalLight";
			case KG::Renderer::ParticleAdd:
				return "ParticleAdd";
			case KG::Renderer::ParticleTransparent:
				return "ParticleTransparent";
            case KG::Renderer::Sprite:
                return "Sprite";
            case KG::Renderer::UI:
                return "UI";
		}
	}


	constexpr const char* ConvertToMacroString( ShaderMeshType target )
	{
		switch ( target )
		{
		case KG::Renderer::ShaderMeshType::StaticMesh: return "STATIC_MESH";
		case KG::Renderer::ShaderMeshType::SkinnedMesh: return "SKINNED_MESH";
		case KG::Renderer::ShaderMeshType::ParticleTransparent: return "PARTICLE_MESH";
		}
	}

	constexpr const char* ConvertToMacroString( ShaderGeometryType target )
	{
		switch ( target )
		{
		case KG::Renderer::ShaderGeometryType::Default: return "GEOMETRY_NORMAL";
		case KG::Renderer::ShaderGeometryType::GeometryCubeMap: return "GEOMETRY_CUBE";
		case KG::Renderer::ShaderGeometryType::GSCubeShadow: return "GEOMETRY_CUBE_SHADOW";
		case KG::Renderer::ShaderGeometryType::GSCascadeShadow: return "GEOMETRY_CASCADE_SHADOW";
		case KG::Renderer::ShaderGeometryType::Light: return "GEOMETRY_LIGHT";
		case KG::Renderer::ShaderGeometryType::SkyBox: return "GEOMETRY_SKYBOX";
		case KG::Renderer::ShaderGeometryType::Particle: return "GEOMETRY_PARTICLE";
		}

	}

	constexpr const char* ConvertToMacroString( ShaderPixelType target )
	{
		switch ( target )
		{
		case KG::Renderer::ShaderPixelType::Deferred: return "PIXEL_NORMAR_DEFERRED";
		case KG::Renderer::ShaderPixelType::GreenWireFrame: return "PIXEL_GREEN_WIREFRAME";
		case KG::Renderer::ShaderPixelType::GeometricCubeMapDeferred: return "PIXEL_GEOMETRIC_DEFERRED";
		case KG::Renderer::ShaderPixelType::Light: return "PIXEL_LIGHT";
		case KG::Renderer::ShaderPixelType::Shadow: return "PIXEL_SHADOW";
		case KG::Renderer::ShaderPixelType::GSCubeShadow: return "PIXEL_GSCUBE_SHADOW";
		case KG::Renderer::ShaderPixelType::Forward: return "PIXEL_FORWARD";
		case KG::Renderer::ShaderPixelType::Transparent: return "PIXEL_TRANSPARENT";
		case KG::Renderer::ShaderPixelType::SkyBox: return "PIXEL_SKYBOX";
		case KG::Renderer::ShaderPixelType::Add: return "PIXEL_ADD";
		}
	}


	constexpr const char* ConvertToEntryString( ShaderTarget target )
	{
		switch ( target )
		{
		case ShaderTarget::CS_5_0:
		case ShaderTarget::CS_5_1:
			return "ComputeShaderFunction";
		case ShaderTarget::VS_5_0:
		case ShaderTarget::VS_5_1:
			return "VertexShaderFunction";
		case ShaderTarget::DS_5_0:
		case ShaderTarget::DS_5_1:
			return "DomainShaderFunction";
		case ShaderTarget::GS_5_0:
		case ShaderTarget::GS_5_1:
			return "GeometryShaderFunction";
		case ShaderTarget::HS_5_0:
		case ShaderTarget::HS_5_1:
			return "HullShaderFunction";
		case ShaderTarget::PS_5_0:
		case ShaderTarget::PS_5_1:
			return "PixelShaderFunction";
		}
	}

	constexpr const char* ConvertToShaderString( ShaderTarget target )
	{
		switch ( target )
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


    enum MaterialType
    {
        Texture,
        FLOAT1,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        COLOR4,
        COLOR3,
        PADDING
    };

    struct MaterialElement
    {
        MaterialType type;
        int offset;
        std::string comment;
        void DrawGUI(Resource::DynamicElementInterface& data);
    };

	class Shader
	{
	protected:
		std::map<std::tuple<ShaderMeshType, ShaderGeometryType, ShaderPixelType, ShaderTesselation>, ID3D12PipelineState*> pso;
		unsigned renderPriority = 0;
		KG::Resource::Metadata::ShaderSetData shaderSetData;
		std::unique_ptr<Resource::DynamicConstantBufferManager> materialBuffer;
		std::map<KG::Utill::HashString, size_t> materialIndex;
		void CreateMaterialBuffer( const KG::Resource::Metadata::ShaderSetData& data );
		ID3D12PipelineState* GetPSO( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel = ShaderTesselation::NormalMesh );
		D3D12_RASTERIZER_DESC CreateRasterizerState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType );
		D3D12_BLEND_DESC CreateBlendState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType );
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType );
		ID3D10Blob* CompileShaderFromMetadata( ShaderTarget shaderTarget, ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, 
			ShaderTesselation tessel = ShaderTesselation::NormalMesh );
	public:
        std::vector<MaterialElement> MaterialDescription;
		Shader( const KG::Resource::Metadata::ShaderSetData& data );
		~Shader();
		void Set( ID3D12GraphicsCommandList* pd3dCommandList, ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel = ShaderTesselation::NormalMesh );
		ID3D12PipelineState* CreatePSO( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel = ShaderTesselation::NormalMesh );

		auto GetRenderPriority() const { return this->renderPriority; }
		auto GetGroup() const { return static_cast<ShaderGroup>( this->shaderSetData.shaderGroup ); }

		size_t GetMaterialIndex( const KG::Utill::HashString& ID );
		bool CheckMaterialLoaded( const KG::Utill::HashString& ID );
		size_t RequestMaterialIndex( const KG::Utill::HashString& ID );
        Resource::DynamicElementInterface GetMaterialElement(const KG::Utill::HashString& ID);
        Resource::DynamicElementInterface GetMaterialElement(UINT index);
    };
}