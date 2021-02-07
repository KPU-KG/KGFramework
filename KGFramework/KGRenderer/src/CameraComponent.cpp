#include "pch.h"
#include <DirectXMath.h>
#include "MathHelper.h"
#include "D3D12Helper.h"
#include "KGDXRenderer.h"
#include "RootParameterIndex.h"
#include "RenderTexture.h"
#include "CameraComponent.h"

using namespace DirectX;

#pragma region CameraComponent

void KG::Component::CameraComponent::RefreshCameraData()
{
	this->CalculateViewMatrix();
	if ( this->ProjDirty )
		this->CalculateProjectionMatrix();
	this->cameraData->cameraWorldPosition = this->transform->GetWorldPosition();
	this->cameraData->look = this->transform->GetWorldLook();

	this->cameraData->viewProjection = KG::Math::Matrix4x4::Multiply( this->cameraData->projection, this->cameraData->view );
	this->cameraData->inverseViewProjection = KG::Math::Matrix4x4::Multiply( this->cameraData->inverseProjection, this->cameraData->inverseView );
}

static constexpr XMFLOAT3 cubeLook[6] =
{
	XMFLOAT3( 1.0f, 0.0f, 0.0f ),
	XMFLOAT3( -1.0f, 0.0f, 0.0f ),
	XMFLOAT3( 0.0f, 1.0f, 0.0f ),
	XMFLOAT3( 0.0f, -1.0f, 0.0f ),
	XMFLOAT3( 0.0f, 0.0f, 1.0f ),
	XMFLOAT3( 0.0f, 0.0f, -1.0f )
};

static constexpr XMFLOAT3 cubeUp[6] =
{
	XMFLOAT3( 0.0f, 1.0f, 0.0f ),
	XMFLOAT3( 0.0f, 1.0f, 0.0f ),
	XMFLOAT3( 0.0f, 0.0f, -1.0f ),
	XMFLOAT3( 0.0f, 0.0f, +1.0f ),
	XMFLOAT3( 0.0f, 1.0f, 0.0f ),
	XMFLOAT3( 0.0f, 1.0f, 0.0f ),
};

void KG::Component::CameraComponent::CalculateViewMatrix()
{
	auto position = this->transform->GetWorldPosition();
	auto look = this->isCubeRenderer ? cubeLook[this->cubeIndex] : this->transform->GetLook();
	auto up = this->isCubeRenderer ? cubeUp[this->cubeIndex] : this->transform->GetUp();
	auto view = DirectX::XMMatrixLookToLH(
		XMLoadFloat3( &position ),
		XMLoadFloat3( &look ),
		XMLoadFloat3( &up )
	);
	DirectX::XMStoreFloat4x4( &this->cameraData->view, XMMatrixTranspose( view ) );
	DirectX::XMStoreFloat4x4( &this->cameraData->inverseView, XMMatrixTranspose( XMMatrixInverse( nullptr, view ) ) );
}

void KG::Component::CameraComponent::CalculateProjectionMatrix()
{
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians( this->fovY ),
		this->aspectRatio,
		this->nearZ,
		this->farZ
	);
	DirectX::XMStoreFloat4x4( &this->cameraData->projection, XMMatrixTranspose( proj ) );
	DirectX::XMStoreFloat4x4( &this->cameraData->inverseProjection, XMMatrixTranspose( XMMatrixInverse( nullptr, proj ) ) );
	this->ProjDirty = false;
}

void KG::Component::CameraComponent::SetDefaultRender()
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->renderTexture->desc.width;
	viewport.Height = this->renderTexture->desc.height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	this->SetViewport( viewport );

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = this->renderTexture->desc.width;
	scissorRect.top = 0;
	scissorRect.bottom = this->renderTexture->desc.height;

	this->SetScissorRect( scissorRect );
}

void KG::Component::CameraComponent::SetCubeRender( int index )
{
	this->isCubeRenderer = true;
	this->cubeIndex = index;
	this->fovY = 90.0f;
	this->aspectRatio = 1.0f;
}

void KG::Component::CameraComponent::OnDestroy()
{
	this->cameraDataBuffer->Unmap( 0, nullptr );
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease( this->cameraDataBuffer );

	if ( this->isRenderTexureCreatedInCamera )
	{
		this->renderTexture->Release();
		delete this->renderTexture;
	}

	IRenderComponent::OnDestroy();
}

void KG::Component::CameraComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new KG::Component::CameraData();
	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
		inst->GetD3DDevice(),
		KG::Renderer::ConstantBufferSize( sizeof( CameraData ) )
	);
	this->cameraDataBuffer->Map( 0, nullptr, (void**)&this->mappedCameraData );
	this->transform = gameObject->GetComponent<TransformComponent>();
	this->ProjDirty = true;
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetView()
{
	this->RefreshCameraData();
	return this->cameraData->view;
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetProj()
{
	this->RefreshCameraData();
	return this->cameraData->projection;
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetViewProj()
{
	this->RefreshCameraData();
	return this->cameraData->viewProjection;
}

DirectX::BoundingFrustum KG::Component::CameraComponent::GetFrustum()
{
	DirectX::BoundingFrustum bf;
	auto proj = this->GetProj();
	auto view = this->GetView();
	DirectX::BoundingFrustum::CreateFromMatrix( bf, XMLoadFloat4x4( &proj ) );
	XMMATRIX inverseViewProj = XMMatrixInverse( nullptr, XMLoadFloat4x4( &view ) );
	bf.Transform( bf, inverseViewProj );
	return bf;
}

void KG::Component::CameraComponent::OnRender( ID3D12GraphicsCommandList* commandList )
{
}

void KG::Component::CameraComponent::SetCameraRender( ID3D12GraphicsCommandList* commandList )
{
	this->RefreshCameraData();
	std::memcpy( this->mappedCameraData, this->cameraData, sizeof( CameraData ) );
	commandList->SetGraphicsRootConstantBufferView( KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress() );

	commandList->RSSetViewports( 1, &this->viewport );
	commandList->RSSetScissorRects( 1, &this->scissorRect );

	TryResourceBarrier( commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		)
	);
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView( this->renderTexture->GetRenderTargetRTVHandle( this->cubeIndex ), clearColor, 0, nullptr );
}

void KG::Component::CameraComponent::EndCameraRender( ID3D12GraphicsCommandList* commandList )
{
	TryResourceBarrier( commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		)
	);
}

void KG::Component::CameraComponent::SetRenderTexture( KG::Renderer::RenderTexture* renderTexture, int index )
{
	this->isRenderTexureCreatedInCamera = false;
	this->cubeIndex = index;
	this->renderTexture = renderTexture;
}

void KG::Component::CameraComponent::InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc )
{
	this->isRenderTexureCreatedInCamera = true;
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize( desc );
}

#pragma endregion


#pragma region CubeCameraComponent

void KG::Component::CubeCameraComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	for ( auto& camera : this->cameras )
		camera.OnCreate( gameObject );

	for ( size_t i = 0; i < 6; i++ )
	{
		cameras[i].SetCubeRender( i );
	}
	//카메라 데이터 업데이트
}

void KG::Component::CubeCameraComponent::OnDestroy()
{
	for ( auto& camera : this->cameras )
		camera.OnDestroy();
}

void KG::Component::CubeCameraComponent::OnRender( ID3D12GraphicsCommandList* commandList )
{
}

void KG::Component::CubeCameraComponent::InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc )
{
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize( desc );

	for ( size_t i = 0; i < 6; i++ )
	{
		this->cameras[i].SetRenderTexture( this->renderTexture, i );
		this->cameras[i].SetDefaultRender();
	}
}

#pragma endregion

#pragma region GSCubeCameraComponent

void KG::Component::GSCubeCameraComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new KG::Component::GSCubeCameraData();

	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
		inst->GetD3DDevice(),
		KG::Renderer::ConstantBufferSize( sizeof( GSCubeCameraData ) )
	);

	this->cameraDataBuffer->Map( 0, nullptr, (void**)&this->mappedCameraData );
	this->transform = gameObject->GetComponent<TransformComponent>();
	this->ProjDirty = true;
}

void KG::Component::GSCubeCameraComponent::OnDestroy()
{
	this->cameraDataBuffer->Unmap( 0, nullptr );
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease( this->cameraDataBuffer );

	this->renderTexture->Release();
	delete this->renderTexture;

	IRenderComponent::OnDestroy();
}

void KG::Component::GSCubeCameraComponent::RefreshCameraData()
{
	this->CalculateViewMatrix();
	if ( this->ProjDirty )
		this->CalculateProjectionMatrix();

	this->cameraData->cameraWorldPosition = this->transform->GetWorldPosition();
	for ( size_t i = 0; i < 6; i++ )
	{
		this->cameraData->look[i].x = cubeLook[i].x;
		this->cameraData->look[i].y = cubeLook[i].y;
		this->cameraData->look[i].z = cubeLook[i].z;
		this->cameraData->look[i].w = 1.0f;
	}
}

void KG::Component::GSCubeCameraComponent::CalculateViewMatrix()
{
	for ( size_t i = 0; i < 6; i++ )
	{
		auto position = this->transform->GetWorldPosition();
		auto look = cubeLook[i];
		auto up = cubeUp[i];
		auto view = DirectX::XMMatrixLookToLH(
			XMLoadFloat3( &position ),
			XMLoadFloat3( &look ),
			XMLoadFloat3( &up )
		);
		DirectX::XMStoreFloat4x4( &this->cameraData->view[i], XMMatrixTranspose( view ) );
		DirectX::XMStoreFloat4x4( &this->cameraData->inverseView[i], XMMatrixTranspose( XMMatrixInverse( nullptr, view ) ) );
	}
}

void KG::Component::GSCubeCameraComponent::CalculateProjectionMatrix()
{
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians( this->fovY ),
		this->aspectRatio,
		this->nearZ,
		this->farZ
	);
	DirectX::XMStoreFloat4x4( &this->cameraData->projection, XMMatrixTranspose( proj ) );
	DirectX::XMStoreFloat4x4( &this->cameraData->inverseProjection, XMMatrixTranspose( XMMatrixInverse( nullptr, proj ) ) );
	this->ProjDirty = false;
}

void KG::Component::GSCubeCameraComponent::SetDefaultRender()
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->renderTexture->desc.width;
	viewport.Height = this->renderTexture->desc.height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	this->SetViewport( viewport );

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = this->renderTexture->desc.width;
	scissorRect.top = 0;
	scissorRect.bottom = this->renderTexture->desc.height;
	this->SetScissorRect( scissorRect );
}

void KG::Component::GSCubeCameraComponent::OnRender( ID3D12GraphicsCommandList* commandList )
{
}

void KG::Component::GSCubeCameraComponent::SetCameraRender( ID3D12GraphicsCommandList* commandList )
{
	this->RefreshCameraData();
	std::memcpy( this->mappedCameraData, this->cameraData, sizeof( GSCubeCameraData ) );
	commandList->SetGraphicsRootConstantBufferView( KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress() );

	commandList->RSSetViewports( 1, &this->viewport );
	commandList->RSSetScissorRects( 1, &this->scissorRect );

	TryResourceBarrier( commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		)
	);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for ( size_t i = 0; i < 6; i++ )
	{
		commandList->ClearRenderTargetView( this->renderTexture->GetRenderTargetRTVHandle( i ), clearColor, 0, nullptr );
	}
}

void KG::Component::GSCubeCameraComponent::EndCameraRender( ID3D12GraphicsCommandList* commandList )
{
	TryResourceBarrier( commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		)
	);
}

void KG::Component::GSCubeCameraComponent::InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc )
{
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize( desc );
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetView( size_t index )
{
	this->RefreshCameraData();
	return this->cameraData->view[index];
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetProj()
{
	this->RefreshCameraData();
	return this->cameraData->projection;
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetViewProj( size_t index )
{
	this->RefreshCameraData();
	return Math::Matrix4x4::Multiply( this->cameraData->view[index], this->cameraData->projection );
}

#pragma endregion
//
//#pragma region GSCascadeCameraComponent
//
//void KG::Component::GSCascadeCameraComponent::OnCreate( KG::Core::GameObject* gameObject )
//{
//	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
//	this->cameraData = new KG::Component::GSCascadeCameraData();
//
//	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
//		inst->GetD3DDevice(),
//		KG::Renderer::ConstantBufferSize( sizeof( GSCascadeCameraData ) )
//	);
//
//	this->cameraDataBuffer->Map( 0, nullptr, (void**)&this->mappedCameraData );
//	this->transform = gameObject->GetComponent<TransformComponent>();
//	this->ProjDirty = true;
//}
//
//void KG::Component::GSCascadeCameraComponent::OnDestroy()
//{
//	this->cameraDataBuffer->Unmap( 0, nullptr );
//	this->mappedCameraData = nullptr;
//	delete this->cameraData;
//	this->cameraData = nullptr;
//	this->transform = nullptr;
//	TryRelease( this->cameraDataBuffer );
//
//	this->renderTexture->Release();
//	delete this->renderTexture;
//
//	IRenderComponent::OnDestroy();
//}
//
//void KG::Component::GSCascadeCameraComponent::RefreshCameraData()
//{
//	this->CalculateViewMatrix();
//	if ( this->ProjDirty )
//		this->CalculateProjectionMatrix();
//
//	this->cameraData->cameraWorldPosition = this->transform->GetWorldPosition();
//	for ( size_t i = 0; i < 6; i++ )
//	{
//		this->cameraData->look[i].x = cubeLook[i].x;
//		this->cameraData->look[i].y = cubeLook[i].y;
//		this->cameraData->look[i].z = cubeLook[i].z;
//		this->cameraData->look[i].w = 1.0f;
//	}
//}
//
//void KG::Component::GSCascadeCameraComponent::CalculateViewMatrix()
//{
//	for ( size_t i = 0; i < 6; i++ )
//	{
//		auto position = this->transform->GetWorldPosition();
//		auto look = cubeLook[i];
//		auto up = cubeUp[i];
//		auto view = DirectX::XMMatrixLookToLH(
//			XMLoadFloat3( &position ),
//			XMLoadFloat3( &look ),
//			XMLoadFloat3( &up )
//		);
//		DirectX::XMStoreFloat4x4( &this->cameraData->view[i], XMMatrixTranspose( view ) );
//		DirectX::XMStoreFloat4x4( &this->cameraData->inverseView[i], XMMatrixTranspose( XMMatrixInverse( nullptr, view ) ) );
//	}
//}
//
//void KG::Component::GSCascadeCameraComponent::CalculateProjectionMatrix()
//{
//	auto proj = DirectX::XMMatrixPerspectiveFovLH(
//		DirectX::XMConvertToRadians( this->fovY ),
//		this->aspectRatio,
//		this->nearZ,
//		this->farZ
//	);
//	DirectX::XMStoreFloat4x4( &this->cameraData->projection, XMMatrixTranspose( proj ) );
//	DirectX::XMStoreFloat4x4( &this->cameraData->inverseProjection, XMMatrixTranspose( XMMatrixInverse( nullptr, proj ) ) );
//	this->ProjDirty = false;
//}
//
//void KG::Component::GSCascadeCameraComponent::SetDefaultRender()
//{
//	D3D12_VIEWPORT viewport;
//	viewport.TopLeftX = 0;
//	viewport.TopLeftY = 0;
//	viewport.Width = this->renderTexture->desc.width;
//	viewport.Height = this->renderTexture->desc.height;
//	viewport.MinDepth = 0;
//	viewport.MaxDepth = 1;
//	this->SetViewport( viewport );
//
//	D3D12_RECT scissorRect;
//	scissorRect.left = 0;
//	scissorRect.right = this->renderTexture->desc.width;
//	scissorRect.top = 0;
//	scissorRect.bottom = this->renderTexture->desc.height;
//	this->SetScissorRect( scissorRect );
//}
//
//DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetView( size_t index )
//{
//	this->RefreshCameraData();
//	return this->cameraData->view[index];
//}
//
//DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetProj( size_t index )
//{
//	this->RefreshCameraData();
//	return this->cameraData->projection;
//}
//
//DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetViewProj( size_t viewIndex, size_t projIndex )
//{
//	this->RefreshCameraData();
//	return Math::Matrix4x4::Multiply( this->cameraData->view[index], this->cameraData->projection );
//}
//
//void KG::Component::GSCascadeCameraComponent::OnRender( ID3D12GraphicsCommandList* commandList )
//{
//}
//
//void KG::Component::GSCascadeCameraComponent::SetCameraRender( ID3D12GraphicsCommandList* commandList )
//{
//	this->RefreshCameraData();
//	std::memcpy( this->mappedCameraData, this->cameraData, sizeof( GSCubeCameraData ) );
//	commandList->SetGraphicsRootConstantBufferView( KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress() );
//
//	commandList->RSSetViewports( 1, &this->viewport );
//	commandList->RSSetScissorRects( 1, &this->scissorRect );
//
//	TryResourceBarrier( commandList,
//		this->renderTexture->BarrierTransition(
//			D3D12_RESOURCE_STATE_RENDER_TARGET,
//			D3D12_RESOURCE_STATE_RENDER_TARGET,
//			D3D12_RESOURCE_STATE_DEPTH_WRITE
//		)
//	);
//
//	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//	for ( size_t i = 0; i < 6; i++ )
//	{
//		commandList->ClearRenderTargetView( this->renderTexture->GetRenderTargetRTVHandle( i ), clearColor, 0, nullptr );
//	}
//}
//
//void KG::Component::GSCascadeCameraComponent::EndCameraRender( ID3D12GraphicsCommandList* commandList )
//{
//	TryResourceBarrier( commandList,
//		this->renderTexture->BarrierTransition(
//			D3D12_RESOURCE_STATE_COMMON,
//			D3D12_RESOURCE_STATE_COMMON,
//			D3D12_RESOURCE_STATE_COMMON
//		)
//	);
//}
//
//void KG::Component::GSCascadeCameraComponent::InitializeRenderTexture( const KG::Renderer::RenderTextureDesc& desc )
//{
//	this->renderTexture = new KG::Renderer::RenderTexture();
//	this->renderTexture->Initialize( desc );
//}
//
//#pragma endregion
