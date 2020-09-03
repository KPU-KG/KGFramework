#pragma once
#include "IndexAllocator.h"
#include "DynamicData.h"
#include "D3D12Helper.h"
#include <d3d12.h>
namespace KG::Resource
{
	struct DynamicConstantBufferManager;
	struct DynamicElementInterface
	{
		friend DynamicConstantBufferManager;
	private:
		DynamicBufferReader dynamicCPU;
		DynamicBufferReader dynamicGPU;
		size_t index;
		DynamicElementInterface( DynamicBufferReader dynamicCPU , DynamicBufferReader dynamicGPU , size_t index)
			:dynamicCPU( dynamicCPU ), dynamicGPU( dynamicGPU ), index(index)
		{

		}
	public:
		template<typename Ty>
		const Ty& Get(size_t offsetByte )
		{
			return this->dynamicCPU.Get<Ty>( index, offsetByte );
		}

		template<typename Ty>
		void Set(size_t offsetByte, const Ty& value )
		{
			this->dynamicCPU.Get<Ty>( index, offsetByte ) = value;
			this->dynamicGPU.Get<Ty>( index, offsetByte ) = value;
		}
	};
	struct DynamicConstantBufferManager
	{
		Utill::IndexBackAllocator allocator;
		ID3D12Resource* buffer = nullptr;
		DynamicBufferReader dynamicCPU;
		DynamicBufferReader dynamicGPU;
		size_t elementSize;
		size_t elementCount;
		ID3D12Device* device;
	public:
		DynamicConstantBufferManager( ID3D12Device* device, size_t elementSize, size_t elementCount )
		{
			this->device = device;
			this->elementSize = elementSize;
			this->elementCount = elementCount;

			dynamicCPU.elementSize = elementSize;
			dynamicGPU.elementSize = elementSize;

			allocator.Resize( elementCount );

			Resize( elementCount );
		}
		~DynamicConstantBufferManager()
		{
			this->buffer->Unmap( 0, nullptr );
			Renderer::TryRelease( this->buffer );
			delete dynamicCPU.buffer;
		}
		void Resize(size_t newElementCount)
		{
			DebugNormalMessage( "ResizeDynamicBuffer" );
			std::byte* newBuffer = new std::byte[elementSize * newElementCount];
			allocator.Resize( newElementCount );

			KG::Renderer::TryRelease( this->buffer );
			this->buffer = Renderer::CreateUploadHeapBuffer( device, elementSize * newElementCount );
			this->buffer->Map( 0, nullptr, (void**)&this->dynamicGPU.buffer );

			if ( this->dynamicCPU.buffer != nullptr )
			{
				std::memcpy( newBuffer, this->dynamicCPU.buffer, elementSize * elementCount );
				std::memcpy( this->dynamicGPU.buffer, this->dynamicCPU.buffer, elementSize * elementCount );
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

		DynamicElementInterface GetElement(size_t index)
		{
			return DynamicElementInterface( this->dynamicCPU, this->dynamicGPU, index );
		}
		
		auto GetBuffer()
		{
			return this->buffer;
		}
	};

}