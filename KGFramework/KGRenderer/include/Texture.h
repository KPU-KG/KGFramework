#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "d3dx12.h"
#include "ResourceMetaData.h"

namespace KG::Renderer
{
	class DescriptorHeapManager;
}

using Microsoft::WRL::ComPtr;
namespace KG::Resource
{
	struct Texture
	{
		Texture( const Metadata::TextureData& data );
		Texture( ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc );

		//임시로 ComPtr 사용 / 수업 진행하여 텍스처 로더 구현을 배우거나 추후에 구현 
		ComPtr<ID3D12Resource> resource = nullptr;
		ComPtr<ID3D12Resource> uploadBuffer = nullptr;
		UINT uploadFenceValue = 0;
		KG::Renderer::DescriptorHeapManager* heapManager = nullptr;

		bool isDestroy = false;
		size_t index;
		Metadata::TextureData metaData;
		void CreateFromMetadata( const Metadata::TextureData& data );
		void CreateFromRuntimeData( ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc );

		//D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
		//D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

		//size_t GetIndex() const;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;

		void CreateDefaultSRVDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC GetDescriptorDesc() const;

		void RequestIndex();
		void UploadToDescriptorHeap();
		void ReleaseFromDescriptorHeap();

		void LoadResource(ID3D12GraphicsCommandList* cmdList);
		void ReleaseResource();

		void Process( ID3D12GraphicsCommandList* cmdList );
	};
}