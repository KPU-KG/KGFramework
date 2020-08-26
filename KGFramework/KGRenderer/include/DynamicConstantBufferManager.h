#pragma once
#include "IndexAllocator.h"
#include "DynamicData.h"
#include "D3D12Helper.h"
#include <d3d12.h>
namespace KG::Resource
{
	struct DynamicConstantBufferManager
	{
		Utill::IndexBackAllocator allocator;
		ID3D12Resource* buffer = nullptr;
		DynamicBufferReader dynamicCPU;
		DynamicBufferReader dynamicGPU;
		size_t elementSize;
		size_t elementCount;
	public:
		DynamicConstantBufferManager( ID3D12Device* device, size_t elementSize, size_t elementCount )
		{
			this->elementSize = elementSize;
			this->elementCount = elementCount;

			allocator.Resize( elementCount );
			dynamicCPU.elementSize = elementSize;
			dynamicGPU.elementSize = elementSize;

			dynamicCPU.buffer = new std::byte[elementSize * elementCount];
			this->buffer = Renderer::CreateUploadHeapBuffer( device, elementSize * elementCount );
			this->buffer->Map( 0, nullptr, (void**)&this->dynamicGPU.buffer );
		}
		~DynamicConstantBufferManager()
		{
			this->buffer->Unmap( 0, nullptr );
			Renderer::TryRelease( this->buffer );
			delete dynamicCPU.buffer;
		}
		void Resize(size_t newElementCount)
		{
			std::byte* newBuffer = new std::byte[elementSize * newElementCount];
			allocator.Resize( newElementCount );
			if ( this->dynamicCPU.buffer != nullptr )
			{
				std::memcpy( newBuffer, this->dynamicCPU.buffer, elementSize * elementCount );
				delete this->dynamicCPU.buffer;
			}
			this->dynamicCPU.buffer = newBuffer;
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

		template<typename Ty>
		const Ty& Get(size_t index, size_t offsetByte)
		{
			return this->dynamicCPU.Get<Ty>( index, offsetByte );
		}

		template<typename Ty>
		void Set( size_t index, size_t offsetByte, const Ty& value)
		{
			return this->dynamicGPU.Get<Ty>( index, offsetByte ) = value;
		}
		
		auto GetBuffer()
		{
			return this->buffer;
		}
	};
}