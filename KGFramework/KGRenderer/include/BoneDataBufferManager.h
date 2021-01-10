#pragma once
#include "IndexAllocator.h"
#include "BoneData.h"
#include "D3D12Helper.h"
#include <d3d12.h>

namespace KG::Resource
{
	struct BoneBufferManager
	{
		Utill::IndexBackAllocator allocator;
		ID3D12Resource* buffer = nullptr;
		BoneData* mappedBuffer;
		size_t elementSize;
		size_t elementCount;
		ID3D12Device* device;
	public:
		BoneBufferManager( ID3D12Device* device, size_t elementCount )
		{
			this->device = device;
			this->elementSize = sizeof( BoneData );
			this->elementCount = elementCount;

			allocator.Resize( elementCount );

			Resize( elementCount );
		}
		~BoneBufferManager()
		{
			this->buffer->Unmap( 0, nullptr );
			Renderer::TryRelease( this->buffer );
		}
		void Resize( size_t newElementCount )
		{
			allocator.Resize( newElementCount );
			KG::Renderer::TryRelease( this->buffer );
			this->buffer = Renderer::CreateUploadHeapBuffer( device, elementSize * newElementCount );
			this->buffer->Map( 0, nullptr, (void**)&this->mappedBuffer );
			this->elementCount = newElementCount;
		}
		size_t RequestEmptyIndex()
		{
			return this->allocator.RequestEmptyIndex();
		}
		void ReleaseIndex( size_t index )
		{
			this->allocator.ReleaseIndex( index );
		}

		void UpdateBoneData( size_t index, const BoneData& data )
		{
			for ( size_t i = 0; i < MAX_COUNT_BONE; i++ )
			{
				this->UpdateBoneData( index, i, data.offsetMatrixs[i] );
			}
		}
		void UpdateBoneData( size_t index, size_t animationIndex, const XMFLOAT4X4& data )
		{
			XMFLOAT4X4 transposed;
			XMStoreFloat4x4( &transposed, XMMatrixTranspose( XMLoadFloat4x4( &data ) ) );
			this->mappedBuffer[index].offsetMatrixs[animationIndex] = transposed;

		}

		auto GetBuffer()
		{
			return this->buffer;
		}
	};

	struct AnimationBufferManager
	{
		Utill::IndexBackAllocator allocator;
		ID3D12Resource* buffer = nullptr;
		AnimationData* mappedBuffer;
		size_t elementSize;
		size_t elementCount;
		ID3D12Device* device;
	public:
		AnimationBufferManager( ID3D12Device* device, size_t elementCount )
		{
			this->device = device;
			this->elementSize = sizeof(AnimationData);
			this->elementCount = elementCount;

			allocator.Resize( elementCount );

			Resize( elementCount );
		}
		~AnimationBufferManager()
		{
			this->buffer->Unmap( 0, nullptr );
			Renderer::TryRelease( this->buffer );
		}
		void Resize( size_t newElementCount )
		{
			DebugNormalMessage( "ResizeDynamicBuffer" );
			std::byte* newBuffer = new std::byte[elementSize * newElementCount];
			allocator.Resize( newElementCount );

			KG::Renderer::TryRelease( this->buffer );
			this->buffer = Renderer::CreateUploadHeapBuffer( device, elementSize * newElementCount );
			this->buffer->Map( 0, nullptr, (void**)&this->mappedBuffer );
			this->elementCount = newElementCount;
		}
		size_t RequestEmptyIndex()
		{
			return this->allocator.RequestEmptyIndex();
		}
		void ReleaseIndex( size_t index )
		{
			this->allocator.ReleaseIndex( index );
		}

		void UpdateAnimationData( size_t index, const AnimationData& data )
		{
			for ( size_t i = 0; i < MAX_COUNT_BONE; i++ )
			{
				this->UpdateAnimationData( index, i, data.currentTransforms[i] );
			}
		}
		void UpdateAnimationData( size_t index, size_t animationIndex, const XMFLOAT4X4& data )
		{
			XMFLOAT4X4 transposed;
			XMStoreFloat4x4( &transposed, XMMatrixTranspose( XMLoadFloat4x4( &data ) ) );
			this->mappedBuffer[index].currentTransforms[animationIndex] = transposed;

		}

		auto GetBuffer()
		{
			return this->buffer;
		}
	};

}