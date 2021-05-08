#include "pch.h"
#include <iterator>
#include "KGDXRenderer.h"
#include "KGShader.h"
#include "D3D12Helper.h"
#include "KGGeometry.h"
#include "Debug.h"
#include "RootParameterIndex.h"

using namespace KG::Renderer;
size_t KG::Renderer::Shader::GetMaterialIndex( const KG::Utill::HashString& ID )
{
	return this->materialIndex.at( ID );
}
bool KG::Renderer::Shader::CheckMaterialLoaded( const KG::Utill::HashString& ID )
{
	return this->materialIndex.count( ID );
}
size_t KG::Renderer::Shader::RequestMaterialIndex( const KG::Utill::HashString& ID )
{
	auto index = this->materialBuffer->RequestEmptyIndex();
	this->materialIndex.emplace( ID, index );
	return index;
}
KG::Resource::DynamicElementInterface KG::Renderer::Shader::GetMaterialElement( const KG::Utill::HashString& ID )
{
	auto index = this->GetMaterialIndex( ID );
	return this->materialBuffer->GetElement( index );
}
D3D12_RASTERIZER_DESC KG::Renderer::Shader::CreateRasterizerState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType )
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroDesc( d3dRasterizerDesc );
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	if ( pixType == ShaderPixelType::Light )
	{
		d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
		d3dRasterizerDesc.DepthClipEnable = false;
	}
	else if ( pixType == ShaderPixelType::GreenWireFrame )
	{
		d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		d3dRasterizerDesc.DepthClipEnable = true;
	}
	else
	{
		d3dRasterizerDesc.CullMode = this->shaderSetData.enableCullBackface ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
		d3dRasterizerDesc.DepthClipEnable = this->shaderSetData.enableDepthCliping;
	}
	if ( pixType == ShaderPixelType::GSCubeShadow || pixType == ShaderPixelType::Shadow  )
	{
		d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		//d3dRasterizerDesc.DepthBias = 1000000;
		//d3dRasterizerDesc.DepthBiasClamp = 0.0f;
		//d3dRasterizerDesc.SlopeScaledDepthBias = 10.5f;
	}
	d3dRasterizerDesc.FrontCounterClockwise = false;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.MultisampleEnable = false;
	d3dRasterizerDesc.AntialiasedLineEnable = false;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	return d3dRasterizerDesc;
}

D3D12_BLEND_DESC KG::Renderer::Shader::CreateBlendState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType )
{
	D3D12_BLEND_DESC d3dBlendDesc;
	ZeroDesc( d3dBlendDesc );

	if ( pixType == ShaderPixelType::Deferred || pixType == ShaderPixelType::SkyBox || pixType == ShaderPixelType::GSCubeShadow || pixType == ShaderPixelType::Shadow )
	{
		d3dBlendDesc.AlphaToCoverageEnable = false;
		d3dBlendDesc.IndependentBlendEnable = false;
		for ( size_t i = 0; i < 4; i++ )
		{
			d3dBlendDesc.RenderTarget[0].BlendEnable = false;
			d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
			d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
			d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
			d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
	}
	else if ( pixType == ShaderPixelType::Light || pixType == ShaderPixelType::Add )
	{
		d3dBlendDesc.AlphaToCoverageEnable = false;
		d3dBlendDesc.IndependentBlendEnable = false;
		d3dBlendDesc.RenderTarget[0].BlendEnable = true;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else if ( pixType == ShaderPixelType::Transparent )
	{
		d3dBlendDesc.AlphaToCoverageEnable = false;
		d3dBlendDesc.IndependentBlendEnable = false;
		d3dBlendDesc.RenderTarget[0].BlendEnable = true;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else
	{
		DebugAssertion( false, L"정의되지 않은 타입의 Blend OP를 사용 중 입니다." );
	}
	return d3dBlendDesc;
}

ID3D12PipelineState* KG::Renderer::Shader::GetPSO( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel )
{
	auto result = this->pso.find( std::make_tuple( meshType, geoType, pixType, tessel ) );
	if ( result == this->pso.end() )
	{
		return this->CreatePSO( meshType, pixType, geoType, tessel );
	}
	else
	{
		return result->second;
	}
}

D3D12_DEPTH_STENCIL_DESC KG::Renderer::Shader::CreateDepthStencilState( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType )
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroDesc( d3dDepthStencilDesc );

	switch ( pixType )
	{
	case ShaderPixelType::GSCubeShadow:
	case ShaderPixelType::Shadow:
	case ShaderPixelType::Deferred:
	{
		d3dDepthStencilDesc.DepthEnable = true;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = false;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
	}
	break;
	case ShaderPixelType::SkyBox:
	{
		d3dDepthStencilDesc.DepthEnable = true;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		d3dDepthStencilDesc.StencilEnable = false;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
	}
	break;
	case ShaderPixelType::Light:
	{
		d3dDepthStencilDesc.DepthEnable = false;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = false;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}
	break;
	case ShaderPixelType::Transparent:
	case ShaderPixelType::Add:
	{
		d3dDepthStencilDesc.DepthEnable = true;
		d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		d3dDepthStencilDesc.StencilEnable = false;
		d3dDepthStencilDesc.StencilReadMask = 0x00;
		d3dDepthStencilDesc.StencilWriteMask = 0x00;
		d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}
	break;
	case ShaderPixelType::Forward:
	default:
		DebugAssertion( false, L"깊이 스텐실 처리방법이  정의되지 않은 타입을 사용 중 입니다." );
		break;
	}


	return d3dDepthStencilDesc;
}

ID3D10Blob* KG::Renderer::Shader::CompileShaderFromMetadata( ShaderTarget shaderTarget, 
	ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel 
)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG_NAME_FOR_SOURCE;
#endif

	ID3DBlob* errorblob;
	ID3DBlob* shaderBlob;
	static std::wstring buffer;
	D3D_SHADER_MACRO macro[] = {
		ConvertToMacroString( meshType ), "",
		ConvertToMacroString( geoType ), "",
		ConvertToMacroString( pixType ), "",
		NULL, NULL };

	buffer.assign( this->shaderSetData.fileDir.begin(), this->shaderSetData.fileDir.end() );
#if defined(_DEBUG)

	ID3DBlob* errorPreProcBlob;
	ID3DBlob* shaderPreProcBlob;

	std::ifstream codeIfs(buffer.c_str());
	std::string source { std::istreambuf_iterator<char>( codeIfs ), std::istreambuf_iterator<char>() };
	HRESULT prehr = ::D3DPreprocess( source.data(), source.size(), this->shaderSetData.fileDir.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		&shaderPreProcBlob,
		&errorPreProcBlob );
	if ( errorPreProcBlob != nullptr )
	{
		DebugErrorMessage( L"셰이더 컴파일 오류 : " << (char*)errorPreProcBlob->GetBufferPointer() );
	}
	else
	{
		std::string FileName =
			std::string( ConvertToMacroString( meshType ) ) + "_" +
			std::string( ConvertToMacroString( geoType ) ) + "_" +
			std::string( ConvertToMacroString( pixType ) ) + "_" +
			this->shaderSetData.fileDir.substr( this->shaderSetData.fileDir.find_last_of( '/' ) + 1 );
		std::string newPath = "Resource/ShaderScript/ShaderDebugCache/" + FileName;
		std::string newSource{ (char*)shaderPreProcBlob->GetBufferPointer() , shaderPreProcBlob->GetBufferSize() };

		while ( true )
		{
			size_t start= newSource.find( '#' );
			if ( start == std::string::npos )
				break;
			size_t end = newSource.find( '\n', start );
			newSource.erase( start, end - start );
		}

		std::ofstream newFile( newPath, std::ios::trunc );
		newFile.write( newSource.c_str(), newSource.size() );
		newFile.close();
		if ( errorPreProcBlob != nullptr )
		{
			DebugErrorMessage( L"셰이더 컴파일 오류 : " << (char*)errorPreProcBlob->GetBufferPointer() );
		}
		buffer.assign( newPath.begin(), newPath.end() );
	}
	HRESULT hr = ::D3DCompileFromFile( buffer.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		ConvertToEntryString( shaderTarget ),
		ConvertToShaderString( shaderTarget ),
		nCompileFlags | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
		0,
		&shaderBlob,
		&errorblob );
#else
	HRESULT hr = ::D3DCompileFromFile( buffer.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		ConvertToEntryString( shaderTarget ),
		ConvertToShaderString( shaderTarget ),
		nCompileFlags | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES,
		0,
		&shaderBlob,
		&errorblob );
#endif


	if ( errorblob != nullptr )
	{
		DebugErrorMessage( L"셰이더 컴파일 오류 : " << (char*)errorblob->GetBufferPointer() );
	}
	return shaderBlob;
}

void KG::Renderer::Shader::CreateMaterialBuffer( const KG::Resource::Metadata::ShaderSetData& data )
{
	auto device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
	auto elementSize = data.materialParameterSize;
	auto elementCount = 50;
	if ( elementSize > 0 )
	{
		DebugNormalMessage( "Create Material Buffer" );
		materialBuffer = std::make_unique<KG::Resource::DynamicConstantBufferManager>( device, elementSize, elementCount );
	}
}

KG::Renderer::Shader::Shader( const KG::Resource::Metadata::ShaderSetData& data )
{
	this->shaderSetData = data;
	this->CreateMaterialBuffer( this->shaderSetData );
}

KG::Renderer::Shader::~Shader()
{
	for ( auto& i : this->pso)
	{
		TryRelease( i.second );
	}
}

void KG::Renderer::Shader::Set( ID3D12GraphicsCommandList* pd3dCommandList, ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel )
{
	auto* pso = this->GetPSO( meshType, pixType, geoType, tessel );
	pd3dCommandList->SetPipelineState( pso );
	if ( this->materialBuffer )
	{
		pd3dCommandList->SetGraphicsRootShaderResourceView( RootParameterIndex::MaterialData, this->materialBuffer->GetBuffer()->GetGPUVirtualAddress() );
	}
}

ID3D12PipelineState* KG::Renderer::Shader::CreatePSO( ShaderMeshType meshType, ShaderPixelType pixType, ShaderGeometryType geoType, ShaderTesselation tessel )
{
	ID3D10Blob* vertexShader = nullptr;
	ID3D10Blob* pixelShader = nullptr;
	ID3D10Blob* domainShader = nullptr;
	ID3D10Blob* hullShader = nullptr;
	ID3D10Blob* geometryShader = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	ZeroDesc( d3dPipelineStateDesc );
	d3dPipelineStateDesc.pRootSignature = KGDXRenderer::GetInstance()->GetGeneralRootSignature();


	//VS
	{
		vertexShader = CompileShaderFromMetadata( ShaderTarget::VS_5_1, meshType, pixType, geoType );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = vertexShader->GetBufferPointer();
		byteCode.BytecodeLength = vertexShader->GetBufferSize();
		d3dPipelineStateDesc.VS = byteCode;
	}

	//PS
	{
		pixelShader = CompileShaderFromMetadata( ShaderTarget::PS_5_1, meshType, pixType, geoType );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = pixelShader->GetBufferPointer();
		byteCode.BytecodeLength = pixelShader->GetBufferSize();
		d3dPipelineStateDesc.PS = byteCode;
	}

	//GS
	if ( geoType == ShaderGeometryType::GeometryCubeMap ||
		geoType == ShaderGeometryType::GSCubeShadow ||
		geoType == ShaderGeometryType::GSCascadeShadow ||
		geoType == ShaderGeometryType::Particle||
		tessel == ShaderTesselation::TesselationMesh )
	{
		geometryShader = CompileShaderFromMetadata( ShaderTarget::GS_5_1, meshType, pixType, geoType );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = geometryShader->GetBufferPointer();
		byteCode.BytecodeLength = geometryShader->GetBufferSize();
		d3dPipelineStateDesc.GS = byteCode;
	}

	//DS
	if ( tessel != ShaderTesselation::NormalMesh )
	{
		domainShader = CompileShaderFromMetadata( ShaderTarget::DS_5_1, meshType, pixType, geoType );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = domainShader->GetBufferPointer();
		byteCode.BytecodeLength = domainShader->GetBufferSize();
		d3dPipelineStateDesc.DS = byteCode;
	}

	//HS
	if ( tessel != ShaderTesselation::NormalMesh )
	{
		hullShader = CompileShaderFromMetadata( ShaderTarget::HS_5_1, meshType, pixType, geoType );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = hullShader->GetBufferPointer();
		byteCode.BytecodeLength = hullShader->GetBufferSize();
		d3dPipelineStateDesc.HS = byteCode;
	}

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState( meshType, pixType, geoType );
	d3dPipelineStateDesc.BlendState = CreateBlendState( meshType, pixType, geoType );
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState( meshType, pixType, geoType );

	d3dPipelineStateDesc.InputLayout = NormalVertex::GetInputLayoutDesc();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	if ( tessel != ShaderTesselation::NormalMesh )
	{
		d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	else if ( geoType == ShaderGeometryType::Particle )
	{
		d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	}
	else 
	{
		d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
	if ( pixType == ShaderPixelType::Deferred || pixType == ShaderPixelType::GreenWireFrame )
	{
		d3dPipelineStateDesc.NumRenderTargets = 4;
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineStateDesc.RTVFormats[2] = DXGI_FORMAT_R16G16_SNORM;
		d3dPipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UINT;
		//d3dPipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else if ( geoType == ShaderGeometryType::GSCubeShadow || geoType == ShaderGeometryType::GSCascadeShadow )
	{
		d3dPipelineStateDesc.NumRenderTargets = 6;
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineStateDesc.RTVFormats[1] = d3dPipelineStateDesc.RTVFormats[0];
		d3dPipelineStateDesc.RTVFormats[2] = d3dPipelineStateDesc.RTVFormats[0];
		d3dPipelineStateDesc.RTVFormats[3] = d3dPipelineStateDesc.RTVFormats[0];
		d3dPipelineStateDesc.RTVFormats[4] = d3dPipelineStateDesc.RTVFormats[0];
		d3dPipelineStateDesc.RTVFormats[5] = d3dPipelineStateDesc.RTVFormats[0];

	}
	else
	{
		d3dPipelineStateDesc.NumRenderTargets = 1;
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	auto d3dDevice = KGDXRenderer::GetInstance()->GetD3DDevice();

	ID3D12PipelineState* pso = nullptr;
	d3dDevice->CreateGraphicsPipelineState( &d3dPipelineStateDesc, IID_PPV_ARGS( &pso ) );

	this->pso.emplace( std::make_tuple( meshType, geoType, pixType, tessel ), pso );

	TryRelease( vertexShader );
	TryRelease( pixelShader );
	TryRelease( domainShader );
	TryRelease( hullShader );
	TryRelease( geometryShader );

	return pso;
}
