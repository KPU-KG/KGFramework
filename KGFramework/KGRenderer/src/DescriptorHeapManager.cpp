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
	this->heapDesc = heapDesc;
	this->numMaxDescriptor = this->heapDesc.NumDescriptors;
	this->descriptorSize = descriptorSize;

	this->allocator.Resize( this->numMaxDescriptor );
	this->device = device;
	device->CreateDescriptorHeap( &this->heapDesc, IID_PPV_ARGS( &this->_heap ) );
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
	if ( this->allocator.isFull() ) 
	{
		this->Resize( this->numMaxDescriptor * 2 );
	}
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

void KG::Renderer::DescriptorHeapManager::Resize( size_t size )
{
	heapDesc.NumDescriptors = size;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	this->descriptorSize = descriptorSize;

	this->allocator.Resize( size );

	ID3D12DescriptorHeap* newHeap;
	device->CreateDescriptorHeap( &heapDesc, IID_PPV_ARGS( &newHeap ) );

	device->CopyDescriptorsSimple( 
		this->numMaxDescriptor, 
		newHeap->GetCPUDescriptorHandleForHeapStart(),
		this->_heap->GetCPUDescriptorHandleForHeapStart(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
	this->numMaxDescriptor = heapDesc.NumDescriptors;
	this->_heap->Release();
	this->_heap = newHeap;
}
