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
	auto position = this->transform->GetPosition();
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
