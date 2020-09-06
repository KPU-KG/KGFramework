#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <array>
#include <vector>
#include <string_view>
#include "D3D12Helper.h"
#include "ResourceMetaData.h"
namespace KG::Renderer
{
	using namespace DirectX;
	struct NormalVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 bitangent;
		XMFLOAT2 uv;
		static const std::array<D3D12_INPUT_ELEMENT_DESC, 5> inputElementDesc;
		static D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc();
	};


	class Geometry
	{
	public:
		Geometry() = default;
		Geometry( const KG::Resource::Metadata::GeometrySetData& metadata );
		virtual ~Geometry();
	protected:
		std::vector<NormalVertex> vertices;
		std::vector<UINT> indices;
		
		ID3D12Resource* vertexBuffer = nullptr;
		ID3D12Resource* indexBuffer = nullptr;

		UINT64 uploadFence = 0;
		ID3D12Resource* vertexUploadBuffer = nullptr;
		ID3D12Resource* indexUploadBuffer = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	public:
		/// @brief 해당 지오메트리를 렌더합니다.
		/// @param commandList 렌더할 커맨드 리스트입니다.
		/// @param nInstance 렌더할 지오메트리의 인스턴스 갯수입니다.
		virtual void Render(ID3D12GraphicsCommandList* commandList, UINT nInstance);
		void Load( ID3D12Device* device, ID3D12GraphicsCommandList* commandList );
		void CreateFromAssimp( const std::string& fileDir );
		void CreateFromMetadata( const KG::Resource::Metadata::GeometrySetData& metadata );
		auto IsLoaded() const
		{
			return this->vertexBuffer != nullptr;
		};
	};
};
