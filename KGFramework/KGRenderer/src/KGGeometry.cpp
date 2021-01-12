#include "pch.h"
#include <vector>
#include "KGGeometry.h"
#include "D3D12Helper.h"
#include "AssimpImpoter.h"
#include "KGDXRenderer.h"

const std::array<D3D12_INPUT_ELEMENT_DESC, 8> KG::Renderer::NormalVertex::inputElementDesc
{
	D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "BONE", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

D3D12_INPUT_LAYOUT_DESC KG::Renderer::NormalVertex::GetInputLayoutDesc()
{
	D3D12_INPUT_LAYOUT_DESC desc;
	ZeroDesc( desc );
	desc.pInputElementDescs = NormalVertex::inputElementDesc.data();
	desc.NumElements = NormalVertex::inputElementDesc.size();
	return desc;
}

KG::Renderer::Geometry::Geometry( const KG::Utill::MeshData& data )
{
	this->CreateFromMeshData( data );
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

	this->indexBuffer = CreateBufferResource( device, commandList, (void*)this->indices.data(), this->indices.size() * sizeof( UINT ),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &this->indexUploadBuffer );

	this->indexBufferView.BufferLocation = this->indexBuffer->GetGPUVirtualAddress();
	this->indexBufferView.SizeInBytes = sizeof( UINT ) * this->indices.size();
	this->indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	this->boneBuffer = CreateBufferResource( device, commandList, (void*)this->bones.offsetMatrixs.data(), this->bones.offsetMatrixs.size() * sizeof( XMFLOAT4X4 ),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &this->boneUploadBuffer );
}

void KG::Renderer::Geometry::CreateFromMeshData( const KG::Utill::MeshData& data )
{
	this->indices = std::move( data.indices );
	this->vertices.resize( data.positions.size() );
	for ( size_t i = 0; i < data.positions.size(); i++ )
	{
		this->vertices[i].position = data.positions[i];
		this->vertices[i].normal = data.normals[i];
		this->vertices[i].tangent = data.tangent[i];
		this->vertices[i].bitangent = data.biTangent[i];
		this->vertices[i].uv0 = data.uvs[0][i];

		if ( data.uvs.size() > 1 )
		{
			this->vertices[i].uv1 = data.uvs[1][i];
		}

		if ( data.bones.size() > 0 )
		{
			auto bone = data.vertexBone[i];
			this->vertices[i].boneId = XMUINT4( bone[0].bondId, bone[1].bondId, bone[2].bondId, bone[3].bondId );
			this->vertices[i].boneWeight = XMFLOAT4( bone[0].boneWeight, bone[1].boneWeight, bone[2].boneWeight, bone[3].boneWeight );
		}
	}

	for ( size_t i = 0; i < data.bones.size(); i++ )
	{
		XMFLOAT4X4 transposed;
		XMStoreFloat4x4( &transposed, XMMatrixTranspose( XMLoadFloat4x4( &data.bones[i].offsetMatrix ) ) );
		this->bones.offsetMatrixs[i] = transposed;
	}
}
