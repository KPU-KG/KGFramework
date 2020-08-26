#include "pch.h"
#include "DescriptorHeapManager.h"
#include "D3D12Helper.h"
#include "d3dx12.h"

KG::Renderer::DescriptorHeapManager::~DescriptorHeapManager()
{
	TryRelease( this->_heap );
}

void KG::Renderer::DescriptorHeapManager::Initialize( ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_DESC heapDesc, size_t descriptorSize )
{
	this->numMaxDescriptor = heapDesc.NumDescriptors;
	this->descriptorSize = descriptorSize;

	this->allocator.Resize( this->numMaxDescriptor );
	device->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( &this->_heap ) );
}

D3D12_CPU_DESCRIPTOR_HANDLE KG::Renderer::DescriptorHeapManager::GetCPUHandle( size_t _index ) const
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle( this->_heap->GetCPUDescriptorHandleForHeapStart() );
	handle.Offset( _index, this->descriptorSize );
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE KG::Renderer::DescriptorHeapManager::GetGPUHandle( size_t _index ) const
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle( this->_heap->GetGPUDescriptorHandleForHeapStart() );
	handle.Offset( _index, this->descriptorSize );
	return handle;
}

size_t KG::Renderer::DescriptorHeapManager::RequestEmptyIndex()
{
	return this->allocator.RequestEmptyIndex();
}

void KG::Renderer::DescriptorHeapManager::ReleaseHandleAtIndex( size_t index )
{
	this->allocator.ReleaseIndex( index );
}

ID3D12DescriptorHeap* KG::Renderer::DescriptorHeapManager::Get() const
{
	return this->_heap;
}

ID3D12DescriptorHeap* const* KG::Renderer::DescriptorHeapManager::GetAddressOf() const
{
	return &this->_heap;
}

ID3D12DescriptorHeap* KG::Renderer::DescriptorHeapManager::operator->() const
{
	return this->Get();
}
