#include "pch.h"
#include <vector>
#include "KGGeometry.h"
#include "D3D12Helper.h"
#include "AssimpImpoter.h"
#include "KGDXRenderer.h"

const std::array<D3D12_INPUT_ELEMENT_DESC, 5> KG::Renderer::NormalVertex::inputElementDesc
{
	D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

D3D12_INPUT_LAYOUT_DESC KG::Renderer::NormalVertex::GetInputLayoutDesc()
{
	D3D12_INPUT_LAYOUT_DESC desc;
	ZeroDesc( desc );
	desc.pInputElementDescs = NormalVertex::inputElementDesc.data();
	desc.NumElements = NormalVertex::inputElementDesc.size();
	return desc;
}

KG::Renderer::Geometry::Geometry( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	this->CreateFromMetadata( metadata );
}

KG::Renderer::Geometry::~Geometry()
{
	TryRelease( this->vertexBuffer );
	TryRelease( this->indexBuffer );

	TryRelease( this->vertexUploadBuffer );
	TryRelease( this->indexUploadBuffer );
}

void KG::Renderer::Geometry::Render( ID3D12GraphicsCommandList* commandList, UINT nInstance )
{
	if ( !this->IsLoaded() )
	{
		this->Load( KGDXRenderer::GetInstance()->GetD3DDevice(), commandList );
	}
	if ( this->vertexUploadBuffer != nullptr && KGDXRenderer::GetInstance()->GetFenceValue() > this->uploadFence + 2 )
	{
		TryRelease( this->indexUploadBuffer );
		TryRelease( this->vertexUploadBuffer );
	}
	commandList->IASetVertexBuffers( 0, 1, &this->vertexBufferView );
	commandList->IASetPrimitiveTopology( this->primitiveTopology );
	if ( this->indexBuffer )
	{
		commandList->IASetIndexBuffer( &this->indexBufferView );
		commandList->DrawIndexedInstanced( this->indices.size(), nInstance, 0, 0, 0 );
	}
	else
	{
		commandList->DrawInstanced( this->vertices.size(), nInstance, 0, 0 );
	}
}

void KG::Renderer::Geometry::Load( ID3D12Device* device, ID3D12GraphicsCommandList* commandList )
{
	this->uploadFence = KGDXRenderer::GetInstance()->GetFenceValue();

	this->vertexBuffer = CreateBufferResource( device, commandList, (void*)this->vertices.data(), this->vertices.size() * sizeof( NormalVertex ),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &this->vertexUploadBuffer );

	this->vertexBufferView.BufferLocation = this->vertexBuffer->GetGPUVirtualAddress();
	this->vertexBufferView.StrideInBytes = sizeof( NormalVertex );
	this->vertexBufferView.SizeInBytes = sizeof( NormalVertex ) * this->vertices.size();

	this->indexBuffer = CreateBufferResource( device, commandList, (void*)this->indices.data(), this->indices.size() * sizeof( size_t ),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &this->indexUploadBuffer );

	this->indexBufferView.BufferLocation = this->indexBuffer->GetGPUVirtualAddress();
	this->indexBufferView.SizeInBytes = sizeof( UINT ) * this->indices.size();
	this->indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void KG::Renderer::Geometry::CreateFromAssimp( const std::string& fileDir )
{
	using KG::Utill::ModelData;
	ModelData data;
	data.LoadModel( fileDir );
	size_t index = 0;
	this->indices = std::move( data.meshes[index].indices );
	this->vertices.resize( data.meshes[index].positions.size() );
	for ( size_t i = 0; i < data.meshes[index].positions.size(); i++ )
	{
		this->vertices[i].position = data.meshes[index].positions[i];
		this->vertices[i].normal = data.meshes[index].normals[i];
		this->vertices[i].tangent = data.meshes[index].tangent[i];
		this->vertices[i].bitangent = data.meshes[index].biTangent[i];
		this->vertices[i].uv = data.meshes[index].uvs[0][i];
	}
}

void KG::Renderer::Geometry::CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata )
{
	this->CreateFromAssimp( metadata.fileDir );
}
