#include "pch.h"
#include "KGGeometry.h"
#include "D3D12Helper.h"
#include "AssimpImpoter.h"

const std::array<D3D12_INPUT_ELEMENT_DESC, 3> KG::Renderer::NormalVertex::inputElementDesc
{
	D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

inline D3D12_INPUT_LAYOUT_DESC KG::Renderer::NormalVertex::GetInputLayoutDesc()
{
	D3D12_INPUT_LAYOUT_DESC desc;
	ZeroDesc(desc);
	desc.pInputElementDescs = NormalVertex::inputElementDesc.data();
	desc.NumElements = NormalVertex::inputElementDesc.size();
	return desc;
}

KG::Renderer::Geometry::Geometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
}

KG::Renderer::Geometry::~Geometry()
{
	TryRelease(this->vertexBuffer);
	TryRelease(this->indexBuffer);

	TryRelease(this->vertexUploadBuffer);
	TryRelease(this->indexUploadBuffer);
}

void KG::Renderer::Geometry::Render(ID3D12GraphicsCommandList* commandList, UINT nInstance)
{
	commandList->IASetVertexBuffers(0, 1, &this->vertexBufferView);
	commandList->IASetPrimitiveTopology(this->primitiveTopology);
	if (this->indexBuffer)
	{
		commandList->IASetIndexBuffer(&this->indexBufferView);
		commandList->DrawIndexedInstanced(this->indices.size(), nInstance, 0, 0, 0);
	}
	else
	{
		commandList->DrawInstanced(this->vertices.size(), nInstance, 0, 0);
	}
}

KG::Renderer::AssimpGeometry::AssimpGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	:Geometry(device, commandList)
{
}

KG::Renderer::AssimpGeometry::AssimpGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& fileDir)
	: Geometry(device, commandList)
{
	this->Load(fileDir);
}

KG::Renderer::AssimpGeometry::~AssimpGeometry()
{
}

void KG::Renderer::AssimpGeometry::Load(const std::string& fileDir, size_t index)
{
	using KG::Utill::ModelData;
	ModelData data;
	data.LoadModel(fileDir);
	this->indices = std::move(data.meshes[index].indices);
	this->vertices.resize(data.meshes[index].positions.size());
	for (size_t i = 0; i < data.meshes[index].positions.size(); i++)
	{
		this->vertices[i].position = data.meshes[index].positions[i];
		this->vertices[i].normal = data.meshes[index].normals[i];
		this->vertices[i].uv = data.meshes[index].uvs[0][i];
	}
}
