#include "pch.h"
#include <DirectXMath.h>
#include "MathHelper.h"
#include "D3D12Helper.h"
#include "KGDXRenderer.h"
#include "RootParameterIndex.h"
#include "RenderTexture.h"
#include "ShadowCasterComponent.h"

using namespace DirectX;

static constexpr XMFLOAT3 cubeLook[6] =
{
	XMFLOAT3( +1.0f, +0.0f, +0.0f ),
	XMFLOAT3( -1.0f, +0.0f, +0.0f ),
	XMFLOAT3( +0.0f, +1.0f, +0.0f ),
	XMFLOAT3( +0.0f, -1.0f, +0.0f ),
	XMFLOAT3( +0.0f, +0.0f, +1.0f ),
	XMFLOAT3( +0.0f, +0.0f, -1.0f )
};

static constexpr XMFLOAT3 cubeUp[6] =
{
	XMFLOAT3( +0.0f, +1.0f, +0.0f ),
	XMFLOAT3( +0.0f, +1.0f, +0.0f ),
	XMFLOAT3( +0.0f, +0.0f, -1.0f ),
	XMFLOAT3( +0.0f, +0.0f, +1.0f ),
	XMFLOAT3( +0.0f, +1.0f, +0.0f ),
	XMFLOAT3( +0.0f, +1.0f, +0.0f )
};

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
		this->cameraData->look[i].x = cubeUp[i].x;
		this->cameraData->look[i].y = cubeUp[i].y;
		this->cameraData->look[i].z = cubeUp[i].z;
		this->cameraData->look[i].w = 1.0f;
	}
}

void KG::Component::GSCubeCameraComponent::CalculateViewMatrix()
{
	for ( size_t i = 0; i < 6; i++ )
	{
		auto position = this->transform->GetPosition();
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
