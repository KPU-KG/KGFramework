#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <array>
#include <vector>
#include <string_view>
#include "D3D12Helper.h"
namespace KG::Renderer
{
	using namespace DirectX;
	struct NormalVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		static const std::array<D3D12_INPUT_ELEMENT_DESC, 3> inputElementDesc;
		static D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc();
	};


	class Geometry
	{
	public:
		Geometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		virtual ~Geometry();
	protected:
		std::vector<NormalVertex> vertices;
		std::vector<UINT> indices;
		
		ID3D12Resource* vertexBuffer = nullptr;
		ID3D12Resource* indexBuffer = nullptr;

		ID3D12Resource* vertexUploadBuffer = nullptr;
		ID3D12Resource* indexUploadBuffer = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	public:
		virtual void Render(ID3D12GraphicsCommandList* commandList, UINT nInstance);
	};

	class AssimpGeometry : public Geometry
	{
	public:
		AssimpGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		AssimpGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const std::string& fileDir);
		virtual ~AssimpGeometry();
		void Load(const std::string& fileDir, size_t index = 0);
	};

};