#include "pch.h"

#include <DirectXMath.h>

#include "debug.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "KGRenderQueue.h"
#include "Transform.h"
#include "MathHelper.h"
#include "GameObject.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "ResourceContainer.h"
#include "D3D12Helper.h"
#include "RootParameterIndex.h"


using namespace KG::Renderer;

void KG::Component::Render3DComponent::OnRender( ID3D12GraphicsCommandList* commadList )
{
}

void KG::Component::Render3DComponent::OnPreRender()
{
	int updateCount = this->renderJob->GetUpdateCount();
	auto mat = Math::Matrix4x4::Transpose( this->transform->GetGlobalWorldMatrix() );
	this->renderJob->objectBuffer->mappedData[updateCount].world = mat;
	this->renderJob->objectBuffer->mappedData[updateCount].materialIndex = this->material->GetMaterialIndex();
}

void KG::Component::Render3DComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->RegisterTransform( gameObject->GetComponent<TransformComponent>() );
	this->RegisterMaterial( gameObject->GetComponent<MaterialComponent>() );
	this->RegisterGeometry( gameObject->GetComponent<GeometryComponent>() );
	auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->material->shaders, this->geometry->geometry );
	this->SetRenderJob( job );
}

void KG::Component::Render3DComponent::SetVisible( bool visible )
{
	if ( this->isVisible == visible )
		return;
	this->isVisible = visible;
	if ( this->isVisible )
	{
		this->renderJob->OnVisibleAdd();
	}
	else
	{
		this->renderJob->OnVisibleRemove();
	}
}

void KG::Component::Render3DComponent::SetRenderJob( KG::Renderer::KGRenderJob* renderJob )
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd( this->isVisible );
}

void KG::Component::Render3DComponent::RegisterTransform( TransformComponent* transform )
{
	this->transform = transform;
}

void KG::Component::Render3DComponent::RegisterMaterial( MaterialComponent* material )
{
	this->material = material;
}

void KG::Component::Render3DComponent::RegisterGeometry( GeometryComponent* geometry )
{
	this->geometry = geometry;
}

void KG::Component::MaterialComponent::InitializeShader( const KG::Utill::HashString& shaderID )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	this->shaders = inst->LoadShader( shaderID );
}

unsigned KG::Component::MaterialComponent::GetMaterialIndex() const
{
	return 0;
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

void KG::Component::GeometryComponent::InitializeGeometry( const KG::Utill::HashString& shaderID )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	this->geometry = inst->LoadGeometry( shaderID );
}


struct KG::Component::CameraComponent::CameraData
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 viewProjection;
	DirectX::XMFLOAT3 cameraWorldPosition;
	float pad0;
	DirectX::XMFLOAT3 look;
	float pad1;
};

void KG::Component::CameraComponent::RefreshCameraData()
{
	this->CalculateViewMatrix();
	if ( this->ProjDirty )
		this->CalculateProjectionMatrix();
	this->cameraData->cameraWorldPosition = this->transform->GetWorldPosition();
	this->cameraData->look = this->transform->GetWorldLook();
	this->cameraData->viewProjection = Math::Matrix4x4::Multiply( this->cameraData->projection, this->cameraData->view );
}

void KG::Component::CameraComponent::CalculateViewMatrix()
{
	auto position = this->transform->GetPosition();
	auto look = this->transform->GetLook();
	auto up = this->transform->GetUp();
	auto view = DirectX::XMMatrixLookToLH(
		XMLoadFloat3( &position ),
		XMLoadFloat3( &look ),
		XMLoadFloat3( &up )
	);
	DirectX::XMStoreFloat4x4( &this->cameraData->view, XMMatrixTranspose( view ) );
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
	this->ProjDirty = false;
}

void KG::Component::CameraComponent::OnDestroy()
{
	this->cameraBuffer->Unmap( 0, nullptr );
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease( this->cameraBuffer );
	IRenderComponent::OnDestroy();
}

void KG::Component::CameraComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new CameraData();
	this->cameraBuffer = CreateUploadHeapBuffer( inst->GetD3DDevice(), sizeof( CameraData ) );
	this->cameraBuffer->Map( 0, nullptr, (void**)&this->mappedCameraData );
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
	commandList->SetGraphicsRootConstantBufferView( RootParameterIndex::CameraData, this->cameraBuffer->GetGPUVirtualAddress() );
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = float( 0 );
	viewport.TopLeftY = float( 0 );
	viewport.Width = float( 1600 );
	viewport.Height = float( 900);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	D3D12_RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = 1600;
	rect.bottom = 900;


	commandList->RSSetViewports( 1, &viewport );
	commandList->RSSetScissorRects( 1, &rect );
}
