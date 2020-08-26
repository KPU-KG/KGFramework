#pragma once
#include "debug.h"
#include "IndexAllocator.h"
#include <d3d12.h>
#include <vector>
namespace KG::Renderer
{

	/// <summary>
	/// 서술자힙을 대신 관리해주는 클래스입니다.
	/// </summary>
	class DescriptorHeapManager
	{
		using this_type = DescriptorHeapManager;
	private:
		/// <summary>
		/// 해당 오브젝트에서 관리하는 서술자 힙입니다.
		/// </summary>
		ID3D12DescriptorHeap* _heap = nullptr;
		/// <summary>
		/// 해당 디바이스에서의 서술자의 크기입니다.
		/// </summary>
		size_t descriptorSize = 0;
		Utill::IndexBackAllocator allocator;
		/// <summary>
		/// 서술자힙에 저장할 수 있는 서술자의 최대 개수입니다.
		/// </summary>
		size_t numMaxDescriptor = 0;
	public:
		DescriptorHeapManager() {};
		~DescriptorHeapManager();

		/// <summary>
		/// 서술자힙 매지너를 초기화합니다.
		/// </summary>
		/// <param name="device">ID3D12Device 인터페이스 포인터입니다.</param>
		/// <param name="heapDesc">생성할 서술자 힙의 명세입니다.</param>
		/// <param name="descriptorSize">해당 디바이스의 서술자 크기입니다.</param>
		void Initialize( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC heapDesc, size_t descriptorSize );

		/// <summary>
		/// 해당 인덱스의 서술자힙 핸들을 구합니다.
		/// </summary>
		/// <param name="_index">구할 핸들의 인덱스입니다.</param>
		/// <returns>해당 인덱스의 서술자 힙 핸들입니다.</returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle( size_t _index ) const;

		/// <summary>
		/// 해당 인덱스의 서술자힙 핸들을 구합니다.
		/// </summary>
		/// <param name="_index">구할 핸들의 인덱스입니다.</param>
		/// <returns>해당 인덱스의 서술자 힙 핸들입니다.</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle( size_t _index ) const;


		/// <summary>
		/// 비어있는 핸들의 인덱스를 구합니다.
		/// </summary>
		/// <returns>비어있는 핸들의 인덱스입니다.</returns>
		size_t RequestEmptyIndex();

		/// <summary>
		/// 해당 인덱스의 핸들을 무효화합니다.
		/// </summary>
		/// <param name="index">무효화할 핸들의 인덱스입니다.</param>
		void ReleaseHandleAtIndex( size_t index );


		/// <summary>
		/// 해당 서술자힙의 인터페이스 포인터를 구합니다.
		/// </summary>
		/// <returns>해당 서술자힙의 인터페이스 포인터입니다.</returns>
		ID3D12DescriptorHeap* Get() const;

		/// <summary>
		/// 해당 서술자힙의 인터페이스 포인터의 포인터 (포인터의 주소)를 구합니다.
		/// </summary>
		/// <returns>해당 서술자힙의 인터페이스 포인터의 포인터 (포인터의 주소)</returns>
		ID3D12DescriptorHeap* const* GetAddressOf() const;

		/// <summary>
		/// 해당 서술자힙의 인터페이스 포인터입니다. Get()과 동일합니다.
		/// </summary>
		/// <seealso cref="DescriptorManager::Get"/>
		/// <returns>해당 서술자힙의 인터페이스 포인터입니다.</returns>
		ID3D12DescriptorHeap* operator-> () const;

	};
}