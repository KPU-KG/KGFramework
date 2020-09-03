#include "pch.h"
#include "KGDXRenderer.h"
#include "D3D12Helper.h"
#include "Debug.h"
#include "Texture.h"

#include "DescriptorHeapManager.h"
#include "DDSTextureLoader.h"


KG::Resource::Texture::Texture( const Metadata::TextureData& data )
{
	this->CreateFromMetadata(data);
}

void KG::Resource::Texture::CreateFromMetadata( const Metadata::TextureData& data )
{
	this->metaData = data;
	this->RequestIndex();
}

D3D12_SHADER_RESOURCE_VIEW_DESC KG::Resource::Texture::GetDescriptorDesc() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = this->resource->GetDesc().Format;
	srvDesc.ViewDimension = static_cast<D3D12_SRV_DIMENSION>(this->metaData.dimension);
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = this->resource->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0;
	return srvDesc;
}

void KG::Resource::Texture::RequestIndex()
{
	auto renderer = KG::Renderer::KGDXRenderer::GetInstance();
	auto descHeap = renderer->GetDescriptorHeapManager();
	this->index = descHeap->RequestEmptyIndex();
}

void KG::Resource::Texture::UploadToDescriptorHeap()
{
	auto renderer = KG::Renderer::KGDXRenderer::GetInstance();
	auto device = renderer->GetD3DDevice();
	auto descHeap = renderer->GetDescriptorHeapManager();
	DebugNormalMessage( L"리소스 뷰 생성" );
	device->CreateShaderResourceView( this->resource.Get(), &this->GetDescriptorDesc(), descHeap->GetCPUHandle( this->index ) );
	DebugNormalMessage( L"리소스 뷰 종료" );
}

void KG::Resource::Texture::ReleaseFromDescriptorHeap()
{
}

void KG::Resource::Texture::LoadResource( ID3D12GraphicsCommandList* cmdList )
{
	DebugNormalMessage( L"텍스처 로딩" )
	static std::wstring buffer;
	buffer.clear();
	buffer.assign( this->metaData.fileDir.begin(), this->metaData.fileDir.end() );
	auto device = KG::Renderer::KGDXRenderer::GetInstance()->GetD3DDevice();
	this->uploadFenceValue = KG::Renderer::KGDXRenderer::GetInstance()->GetFenceValue() + 1;
	HRESULT hr = 
	DirectX::CreateDDSTextureFromFile12(
		device, cmdList,
		buffer.c_str(),
		this->resource,
		this->uploadBuffer
	);
	UploadToDescriptorHeap();
}

void KG::Resource::Texture::ReleaseResource()
{
}

void KG::Resource::Texture::Process( ID3D12GraphicsCommandList* cmdList )
{
	if ( !this->resource )
	{
		DebugNormalMessage(L"리소스 로딩")
		LoadResource( cmdList );
	}
	if ( this->uploadBuffer )
	{
		if ( KG::Renderer::KGDXRenderer::GetInstance()->GetFenceValue() > this->uploadFenceValue + 2 )
		{
			//this->uploadBuffer.Reset();
		}
	}
}
