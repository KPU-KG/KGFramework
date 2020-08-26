#include "pch.h"
#include "KGDXRenderer.h"
#include "KGShader.h"
#include "D3D12Helper.h"
#include "KGGeometry.h"
#include "Debug.h"
#include "RootParameterIndex.h"

using namespace KG::Renderer;
D3D12_RASTERIZER_DESC KG::Renderer::Shader::CreateRasterizerState( const KG::Resource::Metadata::ShaderSetData& data )
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroDesc( d3dRasterizerDesc );
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = data.enableCullBackface ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = false;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = data.enableDepthCliping;
	d3dRasterizerDesc.MultisampleEnable = false;
	d3dRasterizerDesc.AntialiasedLineEnable = false;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	return d3dRasterizerDesc;
}

D3D12_BLEND_DESC KG::Renderer::Shader::CreateBlendState( const KG::Resource::Metadata::ShaderSetData& data )
{
	D3D12_BLEND_DESC d3dBlendDesc;
	ZeroDesc( d3dBlendDesc );

	if ( data.blendOpType == "opaque" )
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
	else if ( data.blendOpType == "lightPass" )
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
	else
	{
		DebugAssertion( false, L"Blend OP " << data.blendOpType.c_str() << L"는 정의되지 않은 Blend OP 입니다." );
	}
	return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC KG::Renderer::Shader::CreateDepthStencilState( const KG::Resource::Metadata::ShaderSetData& data )
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroDesc( d3dDepthStencilDesc );

	switch ( static_cast<ShaderType>(data.shaderType) )
	{
	case ShaderType::Opaque:
	{
		d3dDepthStencilDesc.DepthEnable = true;
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
	case ShaderType::LightPass:
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
	case ShaderType::Transparent:
		break;
	case ShaderType::PostProcess:
		break;
	default:
		DebugAssertion( false, L"Type " << data.blendOpType.c_str() << L"는 깊이 스텐실 처리방법이  정의되지 않은 Type 입니다." );
		break;
	}


	return d3dDepthStencilDesc;
}

ID3D10Blob* KG::Renderer::Shader::CompileShaderFromMetadata( const KG::Resource::Metadata::ShaderCodeData& data )
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* errorblob;
	ID3DBlob* shaderBlob;
	static std::wstring buffer;
	buffer.assign( data.fileDir.begin(), data.fileDir.end() );
	HRESULT hr = ::D3DCompileFromFile( buffer.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, data.entry.c_str(),
		data.type.c_str(), nCompileFlags, 0, &shaderBlob, &errorblob );

	if ( errorblob != nullptr )
	{
		DebugErrorMessage( L"셰이더 컴파일 오류" << (char*)errorblob->GetBufferPointer() );
	}
	return shaderBlob;
}

void KG::Renderer::Shader::CreateMaterialBuffer( const KG::Resource::Metadata::ShaderSetData& data )
{
	auto device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
	auto elementSize = 64;
	auto elementCount = 100;
	materialBuffer = std::make_unique<KG::Resource::DynamicConstantBufferManager>( device, elementSize, elementCount );
}

KG::Renderer::Shader::Shader( const KG::Resource::Metadata::ShaderSetData& data )
{
	this->CreateFromMetadata( data );
}

KG::Renderer::Shader::~Shader()
{
	TryRelease( normalPso );
	TryRelease( wireframePso );
}

void KG::Renderer::Shader::Set( ID3D12GraphicsCommandList* pd3dCommandList )
{
	pd3dCommandList->SetPipelineState( this->isWireFrame ? this->wireframePso : this->normalPso );
	if ( this->materialBuffer )
	{
		pd3dCommandList->SetGraphicsRootConstantBufferView( RootParameterIndex::MaterialData, this->materialBuffer->GetBuffer()->GetGPUVirtualAddress() );
	}
}

void KG::Renderer::Shader::SetWireframe( bool wireframe )
{
	this->isWireFrame = wireframe;
}

void KG::Renderer::Shader::CreateFromMetadata( const KG::Resource::Metadata::ShaderSetData& data )
{
	ID3D10Blob* vertexShader = nullptr;
	ID3D10Blob* pixelShader = nullptr;
	ID3D10Blob* domainShader = nullptr;
	ID3D10Blob* hullShader = nullptr;
	ID3D10Blob* geometryShader = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	ZeroDesc( d3dPipelineStateDesc );
	d3dPipelineStateDesc.pRootSignature = KGDXRenderer::GetInstance()->GetGeneralRootSignature();

	this->renderPriority = data.renderPriority;
	this->shaderType = (ShaderType)data.shaderType;

	//VS
	if ( data.vertexShader.isEnable )
	{
		vertexShader = CompileShaderFromMetadata( data.vertexShader );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = vertexShader->GetBufferPointer();
		byteCode.BytecodeLength = vertexShader->GetBufferSize();
		d3dPipelineStateDesc.VS = byteCode;
	}

	//PS
	if ( data.pixelShader.isEnable )
	{
		pixelShader = CompileShaderFromMetadata( data.pixelShader );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = pixelShader->GetBufferPointer();
		byteCode.BytecodeLength = pixelShader->GetBufferSize();
		d3dPipelineStateDesc.PS = byteCode;
	}

	//GS
	if ( data.geometryShader.isEnable )
	{
		geometryShader = CompileShaderFromMetadata( data.geometryShader );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = geometryShader->GetBufferPointer();
		byteCode.BytecodeLength = geometryShader->GetBufferSize();
		d3dPipelineStateDesc.GS = byteCode;
	}

	//DS
	if ( data.domainShader.isEnable )
	{
		domainShader = CompileShaderFromMetadata( data.domainShader );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = domainShader->GetBufferPointer();
		byteCode.BytecodeLength = domainShader->GetBufferSize();
		d3dPipelineStateDesc.DS = byteCode;
	}

	//HS
	if ( data.hullShader.isEnable )
	{
		hullShader = CompileShaderFromMetadata( data.hullShader );
		D3D12_SHADER_BYTECODE byteCode;
		byteCode.pShaderBytecode = hullShader->GetBufferPointer();
		byteCode.BytecodeLength = hullShader->GetBufferSize();
		d3dPipelineStateDesc.HS = byteCode;
	}

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState( data );
	d3dPipelineStateDesc.BlendState = CreateBlendState( data );
	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState( data );

	d3dPipelineStateDesc.InputLayout = NormalVertex::GetInputLayoutDesc();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	if ( data.shaderType == ShaderType::Opaque )
	{
		d3dPipelineStateDesc.NumRenderTargets = 4;
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
		d3dPipelineStateDesc.RTVFormats[2] = DXGI_FORMAT_R10G10B10A2_UNORM;
		d3dPipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	else 
	{
		d3dPipelineStateDesc.NumRenderTargets = 1;
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	auto d3dDevice = KGDXRenderer::GetInstance()->GetD3DDevice();
	d3dDevice->CreateGraphicsPipelineState( &d3dPipelineStateDesc, IID_PPV_ARGS( &this->normalPso ) );

	d3dPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
	d3dPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	d3dDevice->CreateGraphicsPipelineState( &d3dPipelineStateDesc, IID_PPV_ARGS( &this->wireframePso ) );
	TryRelease( vertexShader );
	TryRelease( pixelShader );
	TryRelease( domainShader );
	TryRelease( hullShader );
	TryRelease( geometryShader );
}
