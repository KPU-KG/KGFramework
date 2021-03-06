#include "pch.h"
#include <vector>
#include "KGGeometry.h"
#include "D3D12Helper.h"
#include "fbxImpoter.h"
#include "KGDXRenderer.h"
#include "RootParameterIndex.h"
#include <future>

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

KG::Renderer::Geometry::Geometry( D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount )
{
	this->CreateFakeGeometry( topology, vertexCount );
}

KG::Renderer::Geometry::Geometry( const std::pair<D3D12_PRIMITIVE_TOPOLOGY, int>& data )
{
	this->CreateFakeGeometry( data.first, data.second );
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

	if ( this->boneOffsetBuffer != nullptr )
	{
		auto addr = this->boneOffsetBuffer->GetGPUVirtualAddress();
		commandList->SetGraphicsRootShaderResourceView( KG::Renderer::GraphicRootParameterIndex::BoneOffsetData, addr );
	}

	if ( this->vertexBuffer )
	{
		commandList->IASetVertexBuffers( 0, 1, &this->vertexBufferView );
	}
	commandList->IASetPrimitiveTopology( this->primitiveTopology );
	if ( this->fakeVertexCount != 0 )
	{
		commandList->DrawInstanced( this->fakeVertexCount, nInstance, 0, 0 );
	}
	else if ( this->indexBuffer )
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
    isLoaded = true;
	this->uploadFence = KGDXRenderer::GetInstance()->GetFenceValue();
    auto f = std::async([this]()->bool {this->CreateAABB(); return true; });
    this->CreateAABB();

	if ( !this->vertices.empty() )
	{
		this->vertexBuffer = CreateBufferResource( device, commandList, (void*)this->vertices.data(), this->vertices.size() * sizeof( NormalVertex ),
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &this->vertexUploadBuffer );

		this->vertexBufferView.BufferLocation = this->vertexBuffer->GetGPUVirtualAddress();
		this->vertexBufferView.StrideInBytes = sizeof( NormalVertex );
		this->vertexBufferView.SizeInBytes = sizeof( NormalVertex ) * this->vertices.size();
	}

	if ( !this->indices.empty() )
	{
		this->indexBuffer = CreateBufferResource( device, commandList, (void*)this->indices.data(), this->indices.size() * sizeof( UINT ),
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &this->indexUploadBuffer );

		this->indexBufferView.BufferLocation = this->indexBuffer->GetGPUVirtualAddress();
		this->indexBufferView.SizeInBytes = sizeof( UINT ) * this->indices.size();
		this->indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}

	if ( this->hasBone )
	{
		this->boneOffsetBuffer = CreateBufferResource( device, commandList, (void*)this->bones.offsetMatrixs.data(), 64 * sizeof( XMFLOAT4X4 ),
			D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &this->boneUploadBuffer );
	}
    auto result = f.get();
    std::cout << "aabb createed" << std::endl;
}

void KG::Renderer::Geometry::CreateAABB()
{
    if(this->vertices.size() > 0)
        DirectX::BoundingBox::CreateFromPoints(this->aabb, this->vertices.size(), (const XMFLOAT3A*)this->vertices.data(), sizeof(NormalVertex));
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
		else
		{
			this->vertices[i].boneId = XMUINT4( 0, 0, 0, 0 );
			this->vertices[i].boneWeight = XMFLOAT4( 1.0f, 0.0f, 0.0f, 0.0f );
		}
	}

	for ( size_t i = 0; i < data.bones.size(); i++ )
	{
		this->hasBone = true;
		this->bones.offsetMatrixs[i] = Math::Matrix4x4::Transpose( data.bones[i].offsetMatrix );
		this->boneIds.push_back(data.bones[i].nodeId);
	}
}

void KG::Renderer::Geometry::CreateFakeGeometry( D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount )
{
	this->primitiveTopology = topology;
	this->fakeVertexCount = vertexCount;
}
