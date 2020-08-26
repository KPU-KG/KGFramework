#pragma once
#include "debug.h"
#include "IndexAllocator.h"
#include <d3d12.h>
#include <vector>
namespace KG::Renderer
{

	/// <summary>
	/// ���������� ��� �������ִ� Ŭ�����Դϴ�.
	/// </summary>
	class DescriptorHeapManager
	{
		using this_type = DescriptorHeapManager;
	private:
		/// <summary>
		/// �ش� ������Ʈ���� �����ϴ� ������ ���Դϴ�.
		/// </summary>
		ID3D12DescriptorHeap* _heap = nullptr;
		/// <summary>
		/// �ش� ����̽������� �������� ũ���Դϴ�.
		/// </summary>
		size_t descriptorSize = 0;
		Utill::IndexBackAllocator allocator;
		/// <summary>
		/// ���������� ������ �� �ִ� �������� �ִ� �����Դϴ�.
		/// </summary>
		size_t numMaxDescriptor = 0;
	public:
		DescriptorHeapManager() {};
		~DescriptorHeapManager();

		/// <summary>
		/// �������� �����ʸ� �ʱ�ȭ�մϴ�.
		/// </summary>
		/// <param name="device">ID3D12Device �������̽� �������Դϴ�.</param>
		/// <param name="heapDesc">������ ������ ���� ���Դϴ�.</param>
		/// <param name="descriptorSize">�ش� ����̽��� ������ ũ���Դϴ�.</param>
		void Initialize( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC heapDesc, size_t descriptorSize );

		/// <summary>
		/// �ش� �ε����� �������� �ڵ��� ���մϴ�.
		/// </summary>
		/// <param name="_index">���� �ڵ��� �ε����Դϴ�.</param>
		/// <returns>�ش� �ε����� ������ �� �ڵ��Դϴ�.</returns>
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle( size_t _index ) const;

		/// <summary>
		/// �ش� �ε����� �������� �ڵ��� ���մϴ�.
		/// </summary>
		/// <param name="_index">���� �ڵ��� �ε����Դϴ�.</param>
		/// <returns>�ش� �ε����� ������ �� �ڵ��Դϴ�.</returns>
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle( size_t _index ) const;


		/// <summary>
		/// ����ִ� �ڵ��� �ε����� ���մϴ�.
		/// </summary>
		/// <returns>����ִ� �ڵ��� �ε����Դϴ�.</returns>
		size_t RequestEmptyIndex();

		/// <summary>
		/// �ش� �ε����� �ڵ��� ��ȿȭ�մϴ�.
		/// </summary>
		/// <param name="index">��ȿȭ�� �ڵ��� �ε����Դϴ�.</param>
		void ReleaseHandleAtIndex( size_t index );


		/// <summary>
		/// �ش� ���������� �������̽� �����͸� ���մϴ�.
		/// </summary>
		/// <returns>�ش� ���������� �������̽� �������Դϴ�.</returns>
		ID3D12DescriptorHeap* Get() const;

		/// <summary>
		/// �ش� ���������� �������̽� �������� ������ (�������� �ּ�)�� ���մϴ�.
		/// </summary>
		/// <returns>�ش� ���������� �������̽� �������� ������ (�������� �ּ�)</returns>
		ID3D12DescriptorHeap* const* GetAddressOf() const;

		/// <summary>
		/// �ش� ���������� �������̽� �������Դϴ�. Get()�� �����մϴ�.
		/// </summary>
		/// <seealso cref="DescriptorManager::Get"/>
		/// <returns>�ش� ���������� �������̽� �������Դϴ�.</returns>
		ID3D12DescriptorHeap* operator-> () const;

	};
}