#include "pch.h"
#include <DirectXMath.h>
#include <array>
#include "MathHelper.h"
#include "D3D12Helper.h"
#include "Scene.h"
#include "KGDXRenderer.h"
#include "RootParameterIndex.h"
#include "RenderTexture.h"
#include "CameraComponent.h"

#include "pix3.h"

using namespace DirectX;

#pragma region CameraComponent

void KG::Component::CameraComponent::RefreshCameraData()
{
	this->CalculateViewMatrix();
	if ( this->ProjDirty )
		this->CalculateProjectionMatrix();
	this->cameraData->cameraWorldPosition = this->transform->GetWorldPosition();
	this->cameraData->look = this->transform->GetWorldLook();

	this->cameraData->viewProjection = KG::Math::Matrix4x4::Multiply(this->cameraData->projection, this->cameraData->view);
	this->cameraData->inverseViewProjection = KG::Math::Matrix4x4::Multiply(this->cameraData->inverseProjection, this->cameraData->inverseView);
}

static constexpr XMFLOAT3 cubeLook[6] =
{
	XMFLOAT3(1.0f, 0.0f, 0.0f),
	XMFLOAT3(-1.0f, 0.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, -1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f)
};

static constexpr XMFLOAT3 cubeUp[6] =
{
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f),
	XMFLOAT3(0.0f, 0.0f, +1.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
	XMFLOAT3(0.0f, 1.0f, 0.0f),
};

KG::Component::CameraComponent::CameraComponent()
	:
	renderTextureProperty(this->renderTextureDesc),
	mainCameraProp("IsMainCamera", this->isMainCamera, true),
	fovYProp("FovY", fovY),
	aspectRatioProp("AspectRatio", aspectRatio, true),
	nearZProp("Near-Z", nearZ, true),
	farZProp("Far-Z", farZ, true)
{
}

void KG::Component::CameraComponent::CalculateViewMatrix()
{
	auto position = this->transform->GetWorldPosition();
	auto look = this->isCubeRenderer ? cubeLook[this->cubeIndex] : this->transform->GetLook();
	auto up = this->isCubeRenderer ? cubeUp[this->cubeIndex] : this->transform->GetUp();
	auto view = DirectX::XMMatrixLookToLH(
		XMLoadFloat3(&position),
		XMLoadFloat3(&look),
		XMLoadFloat3(&up)
	);
	DirectX::XMStoreFloat4x4(&this->cameraData->view, XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&this->cameraData->inverseView, XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
}

void KG::Component::CameraComponent::CalculateProjectionMatrix()
{
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(this->fovY),
		this->aspectRatio,
		this->nearZ,
		this->farZ
	);
	DirectX::XMStoreFloat4x4(&this->cameraData->projection, XMMatrixTranspose(proj));
	DirectX::XMStoreFloat4x4(&this->cameraData->inverseProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, proj)));
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

	this->SetViewport(viewport);

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = this->renderTexture->desc.width;
	scissorRect.top = 0;
	scissorRect.bottom = this->renderTexture->desc.height;

	this->SetScissorRect(scissorRect);
}

void KG::Component::CameraComponent::SetCubeRender(int index)
{
	this->isCubeRenderer = true;
	this->cubeIndex = index;
	this->fovY = 90.0f;
	this->aspectRatio = 1.0f;
}

void KG::Component::CameraComponent::OnDestroy()
{
	this->cameraDataBuffer->Unmap(0, nullptr);
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease(this->cameraDataBuffer);

	if ( this->isRenderTexureCreatedInCamera )
	{
		this->renderTexture->Release();
		delete this->renderTexture;
	}

	IRenderComponent::OnDestroy();
}

void KG::Component::CameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new KG::Component::CameraData();
	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
		inst->GetD3DDevice(),
		KG::Renderer::ConstantBufferSize(sizeof(CameraData))
	);
	this->cameraDataBuffer->Map(0, nullptr, (void**)&this->mappedCameraData);
	this->transform = gameObject->GetComponent<TransformComponent>();
	this->ProjDirty = true;
	if ( this->isCreateRenderTexture )
	{
		this->InitializeRenderTexture();
	}
	if ( this->isMainCamera )
	{
		this->SetMainCamera();
	}
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetView()
{
	return this->cameraData->view;
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetProj()
{
	return this->cameraData->projection;
}

DirectX::XMFLOAT4X4 KG::Component::CameraComponent::GetViewProj()
{
	return this->cameraData->viewProjection;
}

DirectX::BoundingFrustum KG::Component::CameraComponent::GetFrustum()
{
	DirectX::BoundingFrustum bf;
	auto proj = this->GetProj();
	auto view = this->GetView();
	DirectX::BoundingFrustum::CreateFromMatrix(bf, XMMatrixTranspose(XMLoadFloat4x4(&proj)));
	XMMATRIX inverseView = XMMatrixInverse(nullptr, XMMatrixTranspose(XMLoadFloat4x4(&view)));
	bf.Transform(bf, inverseView);
	return bf;
}

void KG::Component::CameraComponent::OnRender(ID3D12GraphicsCommandList* commandList)
{
}

void KG::Component::CameraComponent::OnPreRender()
{
	this->RefreshCameraData();
}

void KG::Component::CameraComponent::SetMainCamera()
{
	this->isMainCamera = true;
	auto* prev = this->gameObject->GetScene()->SetMainCamera(this);
	static_cast<KG::Component::CameraComponent*>(prev)->isMainCamera = false;
}

void KG::Component::CameraComponent::SetCameraRender(ID3D12GraphicsCommandList* commandList)
{
	std::memcpy(this->mappedCameraData, this->cameraData, sizeof(CameraData));
	commandList->SetGraphicsRootConstantBufferView(KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress());

	commandList->RSSetViewports(1, &this->viewport);
	commandList->RSSetScissorRects(1, &this->scissorRect);

	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		)
	);
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if ( this->renderTexture->desc.useRenderTarget )
	{
		commandList->ClearRenderTargetView(this->renderTexture->GetRenderTargetRTVHandle(this->cubeIndex), clearColor, 0, nullptr);
	}
}

void KG::Component::CameraComponent::EndCameraRender(ID3D12GraphicsCommandList* commandList)
{
	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		)
	);
}

void KG::Component::CameraComponent::SetRenderTexture(KG::Renderer::RenderTexture* renderTexture, int index)
{
	this->isRenderTexureCreatedInCamera = false;
	this->cubeIndex = index;
	this->renderTexture = renderTexture;
}

void KG::Component::CameraComponent::InitializeRenderTexture()
{
	this->isRenderTexureCreatedInCamera = true;
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize(this->renderTextureDesc);
}

void KG::Component::CameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->isCreateRenderTexture = true;
	this->mainCameraProp.OnDataLoad(componentElement);
	this->fovYProp.OnDataLoad(componentElement);
	this->aspectRatioProp.OnDataLoad(componentElement);
	this->nearZProp.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
	this->renderTextureProperty.OnDataLoad(componentElement);
}

void KG::Component::CameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::CameraComponent);
	this->mainCameraProp.OnDataSave(componentElement);
	this->fovYProp.OnDataLoad(componentElement);
	this->aspectRatioProp.OnDataLoad(componentElement);
	this->nearZProp.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
	this->renderTextureProperty.OnDataLoad(componentElement);
}

#pragma endregion

#pragma region CubeCameraComponent

void KG::Component::CubeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	for ( auto& directionalLightCamera : this->cameras )
		directionalLightCamera.OnCreate(gameObject);

	for ( size_t i = 0; i < 6; i++ )
	{
		cameras[i].SetCubeRender(i);
	}
	//카메라 데이터 업데이트
}

void KG::Component::CubeCameraComponent::OnDestroy()
{
	for ( auto& directionalLightCamera : this->cameras )
		directionalLightCamera.OnDestroy();
}

KG::Component::CubeCameraComponent::CubeCameraComponent()
	:renderTextureProperty(this->renderTextureDesc)
{
}

void KG::Component::CubeCameraComponent::OnRender(ID3D12GraphicsCommandList* commandList)
{
}

void KG::Component::CubeCameraComponent::OnPreRender()
{
	for ( auto& i : this->cameras )
	{
		i.OnPreRender();
	}
}

void KG::Component::CubeCameraComponent::InitializeRenderTexture()
{
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize(this->renderTextureDesc);

	for ( size_t i = 0; i < 6; i++ )
	{
		this->cameras[i].SetRenderTexture(this->renderTexture, i);
		this->cameras[i].SetDefaultRender();
	}
}

void KG::Component::CubeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->renderTextureProperty.OnDataLoad(componentElement);
}

void KG::Component::CubeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::CubeCameraComponent);
	this->renderTextureProperty.OnDataLoad(componentElement);
}

#pragma endregion

#pragma region GSCubeCameraComponent

void KG::Component::GSCubeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new KG::Component::GSCubeCameraData();

	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
		inst->GetD3DDevice(),
		KG::Renderer::ConstantBufferSize(sizeof(GSCubeCameraData))
	);

	this->cameraDataBuffer->Map(0, nullptr, (void**)&this->mappedCameraData);
	this->transform = gameObject->GetComponent<TransformComponent>();
	this->ProjDirty = true;
}

void KG::Component::GSCubeCameraComponent::OnDestroy()
{
	this->cameraDataBuffer->Unmap(0, nullptr);
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease(this->cameraDataBuffer);

	this->renderTexture->Release();
	delete this->renderTexture;

	IRenderComponent::OnDestroy();
}

KG::Component::GSCubeCameraComponent::GSCubeCameraComponent()
	:
	renderTextureProperty(this->renderTextureDesc),
	nearZProp("Near-Z", nearZ, false),
	farZProp("Far-Z", farZ, false)
{
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
			XMLoadFloat3(&position),
			XMLoadFloat3(&look),
			XMLoadFloat3(&up)
		);
		DirectX::XMStoreFloat4x4(&this->cameraData->view[i], XMMatrixTranspose(view));
		DirectX::XMStoreFloat4x4(&this->cameraData->inverseView[i], XMMatrixTranspose(XMMatrixInverse(nullptr, view)));
	}
}

void KG::Component::GSCubeCameraComponent::CalculateProjectionMatrix()
{
	auto proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(this->fovY),
		this->aspectRatio,
		this->nearZ,
		this->farZ
	);
	DirectX::XMStoreFloat4x4(&this->cameraData->projection, XMMatrixTranspose(proj));
	DirectX::XMStoreFloat4x4(&this->cameraData->inverseProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, proj)));
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
	this->SetViewport(viewport);

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = this->renderTexture->desc.width;
	scissorRect.top = 0;
	scissorRect.bottom = this->renderTexture->desc.height;
	this->SetScissorRect(scissorRect);
}

void KG::Component::GSCubeCameraComponent::OnRender(ID3D12GraphicsCommandList* commandList)
{
}

void KG::Component::GSCubeCameraComponent::OnPreRender()
{
	this->RefreshCameraData();
}

void KG::Component::GSCubeCameraComponent::SetCameraRender(ID3D12GraphicsCommandList* commandList)
{
	std::memcpy(this->mappedCameraData, this->cameraData, sizeof(GSCubeCameraData));
	commandList->SetGraphicsRootConstantBufferView(KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress());

	commandList->RSSetViewports(1, &this->viewport);
	commandList->RSSetScissorRects(1, &this->scissorRect);

	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		)
	);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if ( this->renderTexture->desc.useRenderTarget )
	{
		for ( size_t i = 0; i < 6; i++ )
		{
			commandList->ClearRenderTargetView(this->renderTexture->GetRenderTargetRTVHandle(i), clearColor, 0, nullptr);
		}
	}
}

void KG::Component::GSCubeCameraComponent::EndCameraRender(ID3D12GraphicsCommandList* commandList)
{
	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		)
	);
}

void KG::Component::GSCubeCameraComponent::InitializeRenderTexture()
{
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize(this->renderTextureDesc);
}

void KG::Component::GSCubeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->nearZProp.OnDataLoad(componentElement);
	this->renderTextureProperty.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
}

void KG::Component::GSCubeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::GSCubeCameraComponent);
	this->renderTextureProperty.OnDataLoad(componentElement);
	this->nearZProp.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetView(size_t index)
{
	return this->cameraData->view[index];
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetProj()
{
	return this->cameraData->projection;
}

DirectX::XMFLOAT4X4 KG::Component::GSCubeCameraComponent::GetViewProj(size_t index)
{
	return Math::Matrix4x4::Multiply(this->cameraData->view[index], this->cameraData->projection);
}

#pragma endregion

#pragma region GSCascadeCameraComponent

void KG::Component::GSCascadeCameraComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	auto inst = KG::Renderer::KGDXRenderer::GetInstance();
	this->cameraData = new KG::Component::GSCascadeCameraData();

	this->cameraDataBuffer = KG::Renderer::CreateUploadHeapBuffer(
		inst->GetD3DDevice(),
		KG::Renderer::ConstantBufferSize(sizeof(GSCascadeCameraData))
	);

	this->cameraDataBuffer->Map(0, nullptr, (void**)&this->mappedCameraData);
	this->transform = gameObject->GetComponent<TransformComponent>();
	this->ProjDirty = true;

	this->InitializeRenderTexture();
}

void KG::Component::GSCascadeCameraComponent::OnDestroy()
{
	this->cameraDataBuffer->Unmap(0, nullptr);
	this->mappedCameraData = nullptr;
	delete this->cameraData;
	this->cameraData = nullptr;
	this->transform = nullptr;
	TryRelease(this->cameraDataBuffer);

	this->renderTexture->Release();
	delete this->renderTexture;

	IRenderComponent::OnDestroy();
}

KG::Component::GSCascadeCameraComponent::GSCascadeCameraComponent()
	: 
	renderTextureProperty(renderTextureDesc),
	nearZProp("Near-Z", nearZ, false),
	farZProp("Far-Z", farZ, false)
{
}

void KG::Component::GSCascadeCameraComponent::RefreshCameraData()
{
	this->RefreshCascadeViewProj();
	this->RefreshNormalViewProj();
}

void KG::Component::GSCascadeCameraComponent::SetDefaultRender()
{
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = this->renderTexture->desc.width;
	viewport.Height = this->renderTexture->desc.height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	this->SetViewport(viewport);

	D3D12_RECT scissorRect;
	scissorRect.left = 0;
	scissorRect.right = this->renderTexture->desc.width;
	scissorRect.top = 0;
	scissorRect.bottom = this->renderTexture->desc.height;
	this->SetScissorRect(scissorRect);
}

DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetView(size_t index)
{
	return this->cameraData->view[index];
}

DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetProj(size_t index)
{
	return this->cameraData->projection[index];
}

std::array<DirectX::XMFLOAT4X4, 4> KG::Component::GSCascadeCameraComponent::GetViewProj()
{
	std::array<DirectX::XMFLOAT4X4, 4> arr{};
	for ( size_t i = 0; i < 4; i++ )
	{
		arr[i] = Math::Matrix4x4::Multiply(this->cameraData->projection[i], this->cameraData->view[i]);
	}
	return arr;
}

DirectX::XMFLOAT4X4 KG::Component::GSCascadeCameraComponent::GetViewProj(size_t viewIndex, size_t projIndex)
{
	return Math::Matrix4x4::Multiply(this->cameraData->projection[projIndex], this->cameraData->view[viewIndex]);
}

void KG::Component::GSCascadeCameraComponent::OnRender(ID3D12GraphicsCommandList* commandList)
{
}

void KG::Component::GSCascadeCameraComponent::OnPreRender()
{
	this->RefreshCameraData();
}

void KG::Component::GSCascadeCameraComponent::SetCameraRender(ID3D12GraphicsCommandList* commandList)
{
	std::memcpy(this->mappedCameraData, this->cameraData, sizeof(GSCascadeCameraData));
	auto worldPos = this->mainCamera->GetGameObject()->GetTransform()->GetWorldPosition();
	commandList->SetGraphicsRootConstantBufferView(KG::Renderer::RootParameterIndex::CameraData, this->cameraDataBuffer->GetGPUVirtualAddress());

	commandList->RSSetViewports(1, &this->viewport);
	commandList->RSSetScissorRects(1, &this->scissorRect);

	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		)
	);

	if ( this->renderTexture->desc.useRenderTarget )
	{
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		for ( size_t i = 0; i < this->renderTexture->desc.length; i++ )
		{
			commandList->ClearRenderTargetView(this->renderTexture->GetRenderTargetRTVHandle(i), clearColor, 0, nullptr);
		}
	}
}

void KG::Component::GSCascadeCameraComponent::EndCameraRender(ID3D12GraphicsCommandList* commandList)
{
	TryResourceBarrier(commandList,
		this->renderTexture->BarrierTransition(
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COMMON
		)
	);
}

void KG::Component::GSCascadeCameraComponent::InitalizeCascade(KG::Component::CameraComponent* directionalLightCamera, KG::Component::LightComponent* light)
{
	this->mainCamera = directionalLightCamera;
	this->directionalLight = light;
}

void KG::Component::GSCascadeCameraComponent::RefreshNormalViewProj()
{
	auto worldPos = this->mainCamera->GetGameObject()->GetTransform()->GetWorldPosition();
	DebugNormalMessage("RefreshNormalViewProj : " << worldPos);
	auto lightDirection = this->directionalLight->GetDirectionalLightRef().Direction;
	auto radius = 400.0f;
	auto eyePos = XMVectorSubtract(XMLoadFloat3(&worldPos), XMVectorScale(XMLoadFloat3(&lightDirection), radius));
	auto view = DirectX::XMMatrixLookToLH(
		eyePos,
		XMLoadFloat3(&lightDirection),
		XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f)
	);
	auto proj = DirectX::XMMatrixOrthographicLH(radius, radius, 0.01f, radius * 2.0f);
	XMStoreFloat4(&this->cameraData->cameraWorldPosition[0], eyePos);
	XMStoreFloat4x4(&this->cameraData->view[0], XMMatrixTranspose(view));
	XMStoreFloat4x4(&this->cameraData->projection[0], XMMatrixTranspose(proj));
}

void KG::Component::GSCascadeCameraComponent::RefreshCascadeViewProj()
{
	static constexpr float cascadePoint[] = { 0.0f, 0.07f, 0.4f, 1.0f };

	using namespace Math::Literal;

	auto frustum = mainCamera->GetFrustum();
	XMFLOAT3 corner[8] = {};
	XMFLOAT3 viewCorner[8] = {};
	frustum.GetCorners(corner);
	XMFLOAT3 lightDirection = directionalLight->GetDirectionalLightRef().Direction;
	for ( size_t cascade = 0; cascade < 3; cascade++ )
	{
		std::array<XMFLOAT3, 8> currentCorner;

		for ( size_t c = 0; c < 4; c++ )
		{
			currentCorner[c] = Math::Lerp(corner[c], corner[c + 4], cascadePoint[cascade]);
			currentCorner[c + 4] = Math::Lerp(corner[c], corner[c + 4], cascadePoint[cascade + 1]);
		}

		XMFLOAT3 center = {};
		for ( auto& i : currentCorner )
		{
			center = center + i;
		}
		center = center * (1.0f / 8.0f);

		float radius = 0.0f;
		for ( size_t i = 0; i < 8; i++ )
		{
			float dist = Math::Vector3::Length(currentCorner[i] - center);
			radius = std::max(radius, dist);
		}
		auto eyePos = XMVectorSubtract(XMLoadFloat3(&center), XMVectorScale(XMLoadFloat3(&lightDirection), radius));
		auto view = DirectX::XMMatrixLookToLH(
			eyePos,
			XMLoadFloat3(&lightDirection),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
		);
		float maxX = 0.0f;
		float maxY = 0.0f;
		float maxZ = 0.0f;
		float minX = 100000.0f;
		float minY = 100000.0f;
		float minZ = 100000.0f;

		for ( size_t i = 0; i < 8; i++ )
		{
			auto pos = XMLoadFloat3(&currentCorner[i]);
			auto viewPos = XMVector3TransformCoord(pos, view);
			maxX = std::max(maxX, XMVectorGetX(viewPos));
			maxY = std::max(maxY, XMVectorGetY(viewPos));
			maxZ = std::max(maxZ, XMVectorGetZ(viewPos));
			minX = std::min(minX, XMVectorGetX(viewPos));
			minY = std::min(minY, XMVectorGetY(viewPos));
			minZ = std::min(minZ, XMVectorGetZ(viewPos));
		}


		//auto proj = DirectX::XMMatrixOrthographicLH( radius, radius, 0.01f, radius * 2 );
		auto proj = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, 0.01f, radius * 2);
		XMStoreFloat4(&this->cameraData->cameraWorldPosition[cascade + 1], eyePos);
		XMStoreFloat4x4(&this->cameraData->view[cascade + 1], XMMatrixTranspose(view));
		XMStoreFloat4x4(&this->cameraData->projection[cascade + 1], XMMatrixTranspose(proj));
	}
}

void KG::Component::GSCascadeCameraComponent::InitializeRenderTexture()
{
	this->renderTexture = new KG::Renderer::RenderTexture();
	this->renderTexture->Initialize(this->renderTextureDesc);
}

void KG::Component::GSCascadeCameraComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->nearZProp.OnDataLoad(componentElement);
	this->renderTextureProperty.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
}

void KG::Component::GSCascadeCameraComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::GSCascadeCameraComponent);
	this->renderTextureProperty.OnDataLoad(componentElement);
	this->nearZProp.OnDataLoad(componentElement);
	this->farZProp.OnDataLoad(componentElement);
}

#pragma endregion
