#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <array>
#include <vector>
#include <string_view>
#include "D3D12Helper.h"
#include "ResourceMetaData.h"
#include "BoneData.h"
#include "hash.h"
namespace KG::Utill
{
	class MeshData;
}
namespace KG::Component
{
	class BoneTransformComponent;
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
		friend KG::Component::BoneTransformComponent;
	public:
		Geometry() = default;
		Geometry( const KG::Utill::MeshData& data );
		Geometry( D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount );
		Geometry( const std::pair<D3D12_PRIMITIVE_TOPOLOGY, int>& data );

		virtual ~Geometry();
	protected:
		std::vector<NormalVertex> vertices;
		std::vector<UINT> indices;
		std::vector<KG::Utill::HashString> boneIds;
		KG::Resource::BoneData bones;
		bool hasBone = false;
		int fakeVertexCount = 0;
		
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
        ID3D12Resource* blasScratch = nullptr;
        ID3D12Resource* blasResult = nullptr;
        std::wstring debugName = L"NONAME";
    private:
        bool isLoaded = false;
        bool isLoadedDXR = false;
	public:
        DirectX::BoundingBox aabb;
		/// @brief 해당 지오메트리를 렌더합니다.
		/// @param commandList 렌더할 커맨드 리스트입니다.
		/// @param nInstance 렌더할 지오메트리의 인스턴스 갯수입니다.
		virtual void Render(ID3D12GraphicsCommandList* commandList, UINT nInstance);
		void Load( ID3D12Device* device, ID3D12GraphicsCommandList* commandList );
        void LoadToDXR(ID3D12Device5* device, ID3D12GraphicsCommandList4* commandList);
        void CreateAABB();
		void CreateFromMeshData( const KG::Utill::MeshData& data );
		void CreateFakeGeometry( D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount );
        D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferGPUAddress() const;
        D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferGPUAddress() const;
        auto isFake() const
        {
            return this->fakeVertexCount;
        }
        auto GetCounts() const
        {
            return std::make_pair(this->vertices.size(), this->indices.size());
        }
		auto IsLoaded() const
		{
			return isLoaded;
		};
        auto IsLoadedDXR() const
        {
            return isLoadedDXR;
        };
		auto HasBone() const
		{
			return this->hasBone;
		};
        auto GetBLAS() const
        {
            return this->blasResult->GetGPUVirtualAddress();
        }
	};
};
