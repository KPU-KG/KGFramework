#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <array>
#include <vector>
#include <string_view>
#include "D3D12Helper.h"
#include "ResourceMetaData.h"
#include "BoneData.h"
namespace KG::Utill
{
	class MeshData;
}
namespace KG::Renderer
{
	using namespace DirectX;

	struct NormalVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 bitangent;
		XMFLOAT2 uv0;
		XMFLOAT2 uv1;
		XMUINT4 boneId;
		XMFLOAT4 boneWeight;
		static const std::array<D3D12_INPUT_ELEMENT_DESC, 8> inputElementDesc;
		static D3D12_INPUT_LAYOUT_DESC GetInputLayoutDesc();
	};


	class Geometry
	{
	public:
		Geometry() = default;
		Geometry( const KG::Utill::MeshData& data );
		virtual ~Geometry();
	protected:
		std::vector<NormalVertex> vertices;
		std::vector<UINT> indices;
		KG::Resource::BoneData bones;
		bool hasBone = false;
		
		ID3D12Resource* vertexBuffer = nullptr;
		ID3D12Resource* indexBuffer = nullptr;
		ID3D12Resource* boneOffsetBuffer = nullptr;


		UINT64 uploadFence = 0;
		ID3D12Resource* vertexUploadBuffer = nullptr;
		ID3D12Resource* indexUploadBuffer = nullptr;
		ID3D12Resource* boneUploadBuffer = nullptr;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	public:
		/// @brief �ش� ������Ʈ���� �����մϴ�.
		/// @param commandList ������ Ŀ�ǵ� ����Ʈ�Դϴ�.
		/// @param nInstance ������ ������Ʈ���� �ν��Ͻ� �����Դϴ�.
		virtual void Render(ID3D12GraphicsCommandList* commandList, UINT nInstance);
		void Load( ID3D12Device* device, ID3D12GraphicsCommandList* commandList );
		void CreateFromMeshData( const KG::Utill::MeshData& data );
		auto IsLoaded() const
		{
			return this->vertexBuffer != nullptr;
		};
	};
};
