#pragma once
#include <d3d12.h>
#include <map>
#include <array>
#include <algorithm>
#include <list>
#include "D3D12Helper.h"
#include "ShaderData.h"
namespace KG::Renderer
{
	struct UploadBuffer
	{
		size_t bufferSize = 0;
		ID3D12Resource* resource = nullptr;
		UploadBuffer() = default;
		UploadBuffer(ID3D12Device* device, size_t size)
		{
			this->Initialize(device, size);
		}
		~UploadBuffer()
		{
			this->Release();
		}
		void Initialize(ID3D12Device* device, size_t size)
		{
			if (this->resource)
			{
				if (this->bufferSize != size)
				{
					this->Release();
				}
				else
				{
					return;
				}
			}
			this->bufferSize = size;
			this->resource = CreateUploadHeapBuffer(device, size);
		}
		void Release()
		{
			TryRelease(this->resource);
		}
	};


	template <class Ty>
	struct PooledBuffer
	{
		bool isUsing = false;
		UploadBuffer buffer;
		Ty* mappedData = nullptr;
		PooledBuffer(ID3D12Device* device, size_t count)
			:buffer(device, count * sizeof(Ty))
		{
			this->buffer.resource->Map(0, nullptr, (void**)&this->mappedData);
		}
		size_t GetSize() const
		{
			return this->buffer.bufferSize / sizeof(Ty);
		}
	};

	template<class Ty>
	struct SameCountBufferPool
	{
		using BufferType = PooledBuffer<Ty>;

		size_t count; // 해당 버퍼가 보유한 ObjectData의 갯수
		ID3D12Device* device = nullptr;
		std::list<BufferType> bufferPool;

		SameCountBufferPool(ID3D12Device* device, size_t count, size_t reserved = 0)
			:device(device), count(count)
		{
			if (reserved != 0)
			{
				this->Reserve(reserved);
			}
		};
		void Reserve(size_t size)
		{
			while (this->bufferPool.size() < size)
			{
				this->CreateNewBuffer();
			}
		}
		BufferType& CreateNewBuffer()
		{
			return this->bufferPool.emplace_back(device, count);
		}
		BufferType* GetNewBuffer()
		{
			auto result = std::find_if(
				this->bufferPool.begin(), this->bufferPool.end(),
				[]( BufferType& a) { return a.isUsing == false; }
			);
			if (result == this->bufferPool.end())
			{
				return &this->CreateNewBuffer();
			}
			else
			{
				return &*result;
			}
		}
	};


	template<class Ty, size_t fixed_pool = 11>
	struct BufferPool
	{
		constexpr static std::array<size_t, 11> defaultFixedSize = { 1, 4, 16, 32, 64, 128, 256, 512, 1024, 2048, 100000 };
		constexpr static std::array<size_t, 11> defaultReservedSize = { 100, 50, 25, 15, 10 , 10, 10, 5, 5, 1 };
		ID3D12Device* device = nullptr;
		std::map<size_t, SameCountBufferPool<Ty>> pool;
		std::array<size_t, fixed_pool> fixedSize;
		std::array<size_t, fixed_pool> reservedSize;

		BufferPool(ID3D12Device* device, std::array<size_t, fixed_pool> fixedSize, std::array<size_t, fixed_pool> reservedSize)
			:device(device), fixedSize(fixedSize), reservedSize(reservedSize)
		{
			for (size_t i = 0; i < fixed_pool; i++)
			{
				auto curSize = fixedSize[i];
				auto curReserve = reservedSize[i];
				pool.emplace(
					std::make_pair(curSize, SameCountBufferPool<Ty>(device, curSize, curReserve))
				);
			}
		}
		size_t GetSize(size_t size) const
		{
			auto it = std::lower_bound(fixedSize.begin(), fixedSize.end(), size);
			if (it == fixedSize.end())
			{
				return size;
			}
			else 
			{
				return *it;
			}
		}
		PooledBuffer<Ty>* GetNewBuffer(size_t count)
		{
			size_t bufferSize = this->GetSize(count);
			if ( pool.count( bufferSize ) == 0 )
			{
				pool.emplace(
					std::make_pair( bufferSize, SameCountBufferPool<Ty>( device, bufferSize, 1 ) )
				);
			}
			auto buffer = pool.at( bufferSize ).GetNewBuffer();
			buffer->isUsing = true;
			return buffer;
		}
	};
};