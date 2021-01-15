#include "pch.h"

#include <DirectXMath.h>
#include "d3dx12.h"
#include "debug.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "KGRenderQueue.h"
#include "Transform.h"
#include "MathHelper.h"
#include "GameObject.h"
#include "KGRenderer.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "ResourceContainer.h"
#include "KGResourceLoader.h"
#include "D3D12Helper.h"
#include "RootParameterIndex.h"
#include "RenderTexture.h"


using namespace KG::Renderer;

#pragma region Render3DComponent

void KG::Component::Render3DComponent::OnRender( ID3D12GraphicsCommandList* commadList )
{
}

void KG::Component::Render3DComponent::OnPreRender()
{
	for ( size_t i = 0; i < this->renderJobs.size(); i++ )
	{
		auto* renderJob = this->renderJobs[i];
		int updateCount = renderJob->GetUpdateCount();
		auto mat = Math::Matrix4x4::Transpose( this->transform->GetGlobalWorldMatrix() );
		renderJob->objectBuffer->mappedData[updateCount].object.world = mat;
		if ( this->material )
		{
			renderJob->objectBuffer->mappedData[updateCount].object.materialIndex = this->material->GetMaterialIndex(this->jobMaterialIndexs[i]);
		}
		if ( this->reflectionProbe )
		{
			renderJob->objectBuffer->mappedData[updateCount].object.environmentMapIndex =
				this->reflectionProbe->GetRenderTexture().renderTargetSRVIndex;
		}
		else 
		{
			renderJob->objectBuffer->mappedData[updateCount].object.environmentMapIndex =
				KG::Resource::ResourceContainer::GetInstance()->LoadTexture( KG::Renderer::KGDXRenderer::GetInstance()->GetSkymapTexutreId() )->index;
		}
	}
}

void KG::Component::Render3DComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->RegisterTransform( gameObject->GetComponent<TransformComponent>() );
	this->RegisterMaterial( gameObject->GetComponent<MaterialComponent>() );
	this->RegisterGeometry( gameObject->GetComponent<GeometryComponent>() );
	auto geometryCount = this->geometry->geometrys.size();
	auto materialCount = this->material->materialIndexs.size();

	if ( materialCount != 1 && geometryCount != materialCount )
	{
		DebugErrorMessage( "Material Count Not Matched Geometry" );
	}
	for ( size_t i = 0; i < geometryCount; i++ )
	{
		auto materialIndex = materialCount == 1 ? 0 : i;
		auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->material->shaders[i], this->geometry->geometrys[i] );
		this->AddRenderJob( job, materialIndex );
	}
	//조건문 넣고 렌더잡 만들자
}

void KG::Component::Render3DComponent::SetVisible( bool visible )
{
	for ( auto* renderJob : this->renderJobs )
	{
		if ( this->isVisible == visible )
			return;
		this->isVisible = visible;
		if ( this->isVisible )
		{
			renderJob->OnVisibleAdd();
		}
		else
		{
			renderJob->OnVisibleRemove();
		}
	}
}

void KG::Component::Render3DComponent::SetReflectionProbe( CubeCameraComponent* probe )
{
	this->reflectionProbe = probe;
}

void KG::Component::Render3DComponent::AddRenderJob( KG::Renderer::KGRenderJob* renderJob, UINT materialIndex )
{
	this->renderJobs.push_back( renderJob );
	this->jobMaterialIndexs.push_back( materialIndex );
	renderJob->OnObjectAdd( this->isVisible );
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

#pragma endregion

#pragma region MaterialComponent

void KG::Component::MaterialComponent::InitializeMaterial( const KG::Utill::HashString& materialID, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	auto [index, shaderId] = inst->LoadMaterial( materialID );
	if ( this->materialIndexs.size() < slotIndex + 1 )
	{
		this->materialIndexs.resize( slotIndex + 1 );
	}
	this->materialIndexs[slotIndex] = index;
	this->InitializeShader( shaderId, slotIndex );
}

void KG::Component::MaterialComponent::InitializeShader( const KG::Utill::HashString& shaderID, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->shaders.size() < slotIndex + 1 )
	{
		this->shaders.resize( slotIndex + 1 );
	}
	this->shaders[slotIndex] = inst->LoadShader( shaderID );
}

unsigned KG::Component::MaterialComponent::GetMaterialIndex( UINT slotIndex ) const
{
	return this->materialIndexs.size() != 0 ? this->materialIndexs[slotIndex] : 0;
}

void KG::Component::MaterialComponent::OnDestroy()
{
	IRenderComponent::OnDestroy();
}

#pragma endregion

#pragma region GeometryComponent

void KG::Component::GeometryComponent::InitializeGeometry( const KG::Utill::HashString& geometryID, UINT subMeshIndex, UINT slotIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	if ( this->geometrys.size() < slotIndex + 1 )
	{
		this->geometrys.resize( slotIndex + 1 );
	}
	this->geometrys[slotIndex] =  inst->LoadGeometry( geometryID, subMeshIndex );
}

#pragma endregion

#pragma region CameraComponent

struct KG::Component::CameraComponent::CameraData
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 viewProjection;

	DirectX::XMFLOAT4X4 inverseView;
	DirectX::XMFLOAT4X4 inverseProjection;
	DirectX::XMFLOAT4X4 inverseViewProjection;

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
	this->cameraData = new CameraData();
	this->cameraDataBuffer = CreateUploadHeapBuffer( inst->GetD3DDevice(), ConstantBufferSize( sizeof( CameraData ) ) );
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
	commandList->SetGraphicsRootConstantBufferView( RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress() );

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
	commandList->ClearRenderTargetView( this->renderTexture->GetRenderTargetRTVHandle(this->cubeIndex), clearColor, 0, nullptr );
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

#pragma region LightComponent

void KG::Component::LightComponent::SetRenderJob( KG::Renderer::KGRenderJob* renderJob )
{
	this->renderJob = renderJob;
	this->renderJob->OnObjectAdd( this->isVisible );
}

void KG::Component::LightComponent::RegisterTransform( TransformComponent* transform )
{
	this->transform = transform;
}

void KG::Component::LightComponent::SetDirectionalLight( const DirectX::XMFLOAT3& strength, const DirectX::XMFLOAT3& direction )
{
	isDirty = true;
	this->light.Direction = Math::Vector3::Normalize( direction );
	this->light.Strength = strength;
	if ( this->directionalLightShader == nullptr )
	{
		this->directionalLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "directionalLight"_id ) );
	}
	if ( this->directionalLightGeometry == nullptr )
	{
		this->directionalLightGeometry = KG::Resource::ResourceContainer::GetInstance()->LoadGeometry( Utill::HashString( "lightPlane"_id ) );
	}
	this->currentShader = this->directionalLightShader;
	this->currentGeometry = this->directionalLightGeometry;
}

void KG::Component::LightComponent::SetPointLight( const DirectX::XMFLOAT3& strength, float fallOffStart, float fallOffEnd )
{
	isDirty = true;
	this->light.Strength = strength;
	this->light.FalloffStart = fallOffStart;
	this->light.FalloffEnd = fallOffEnd;
	if ( this->pointLightShader == nullptr )
	{
		this->pointLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "pointLight"_id ) );
	}
	if ( this->pointLightGeometry == nullptr )
	{
		this->pointLightGeometry = KG::Resource::ResourceContainer::GetInstance()->LoadGeometry( Utill::HashString( "sphere"_id ) );
	}
	this->currentShader = this->pointLightShader;
	this->currentGeometry = this->pointLightGeometry;
}

KG::Component::DirectionalLightRef KG::Component::LightComponent::GetDirectionalLightRef()
{
	this->UpdateChanged();
	return DirectionalLightRef( this->light );
}

KG::Component::PointLightRef KG::Component::LightComponent::GetPointLightRef()
{
	this->UpdateChanged();
	return PointLightRef( this->light );
}

void KG::Component::LightComponent::UpdateChanged()
{
	this->isDirty = true;
}

void KG::Component::LightComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->RegisterTransform( gameObject->GetComponent<TransformComponent>() );
	auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->currentShader, this->currentGeometry );
	this->SetRenderJob( job );
}

void KG::Component::LightComponent::OnRender( ID3D12GraphicsCommandList* commadList )
{

}

void KG::Component::LightComponent::OnPreRender()
{
	if ( this->isDirty )
	{
		this->isDirty = false;
		int updateCount = this->renderJob->GetUpdateCount();
		this->light.Position = this->transform->GetWorldPosition();
		std::memcpy( &this->renderJob->objectBuffer->mappedData[updateCount].light, &this->light, sizeof( this->light ) );
	}
}

void KG::Component::LightComponent::SetVisible( bool visible )
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

#pragma endregion

void KG::Component::AvatarComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->geometry = gameObject->GetComponent<KG::Component::GeometryComponent>();

	this->frameCache.resize( this->geometry->geometrys.size() );
	for ( size_t i = 0; i < this->geometry->geometrys.size(); i++ )
	{
		auto& boneIds = this->geometry->geometrys[i]->boneIds;
		auto& cache = this->frameCache[i];
		cache.resize( boneIds.size() );
		gameObject->MatchBoneToObject( boneIds, cache );
	}
}

KG::Core::GameObject* KG::Component::AvatarComponent::BoneIndexToGameObject( UINT index, UINT submeshIndex ) const
{
	return this->frameCache[submeshIndex][index];
}
