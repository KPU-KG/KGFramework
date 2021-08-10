#pragma once
#include <exception>
#include <d3d12.h>
#include <pix3.h>
#include <stdio.h>
#include <stdlib.h>
//#include <PIXEventsCommon.h>



namespace KG::Renderer
{
	template<typename Ty>
	inline void TryRelease(Ty*& ptr)
	{
		if (ptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	};

	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception( HRESULT hr ) : result( hr ) {}

		const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s( s_str, "Failure with HRESULT of %08X",
				static_cast<unsigned int>(result) );
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed( HRESULT hr )
	{
		if ( FAILED( hr ) )
		{
			throw com_exception( hr );
		}
	}

	template<typename Ty>
	inline void ZeroDesc(Ty& desc)
	{
		::ZeroMemory(&desc, sizeof(Ty));
	}

	template<typename Ty>
	inline Ty ConstantBufferSize(Ty value)
	{
		return (value + 255) & ~255;
	}

	ID3D12Resource* CreateBufferResource(
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		void* pData,
		UINT nBytes,
		D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		ID3D12Resource** ppd3dUploadBuffer = nullptr
	);

	ID3D12Resource* CreateRenderTargetResource(
		ID3D12Device* pd3dDevice,
		size_t width,
		size_t height,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM
	);

	ID3D12Resource* CreateArrayRenderTargetResource(
		ID3D12Device* pd3dDevice,
		size_t width,
		size_t height,
		size_t length,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM
	);

	ID3D12Resource* CreateDepthStencilResource(
		ID3D12Device* pd3dDevice,
		size_t width,
		size_t height,
		DXGI_FORMAT format = DXGI_FORMAT_R24G8_TYPELESS
	);

	ID3D12Resource* CreateArrayDepthStencilResource(
		ID3D12Device* pd3dDevice,
		size_t width,
		size_t height,
		size_t length,
		DXGI_FORMAT format = DXGI_FORMAT_R24G8_TYPELESS
	);

    ID3D12Resource* CreateUAVBufferResource(
        ID3D12Device* pd3dDevice,
        UINT width,
        UINT height,
        D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_COPY_SOURCE
    );
    ID3D12Resource* CreateUAVBufferResource(
        ID3D12Device* pd3dDevice,
        UINT width,
        UINT height,
        DXGI_FORMAT format,
        D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_COPY_SOURCE
    );

	ID3D12Resource* CreateUploadHeapBuffer(ID3D12Device* device, size_t bufferSize);
};

using KG::Renderer::ThrowIfFailed;
using KG::Renderer::TryRelease;
