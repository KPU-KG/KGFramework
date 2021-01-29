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
#include "fbximpoter.h"


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
			renderJob->objectBuffer->mappedData[updateCount].object.materialIndex = this->material->GetMaterialIndex( this->jobMaterialIndexs[i] );
		}
		if ( this->boneAnimation && renderJob->animationBuffer != nullptr )
		{
			for ( size_t k = 0; k < this->boneAnimation->frameCache[i].size(); k++ )
			{
				//auto finalAnim = Math::Matrix4x4::Inverse(this->boneAnimation->frameCache[i][k]->GetTransform()->GetGlobalWorldMatrix());
				if ( this->boneAnimation->frameCache[i][k] )
				{
					//auto wo = Math::Matrix4x4::Inverse( this->transform->GetLocalWorldMatrix() );
					auto finalTransform = this->boneAnimation->frameCache[i][k]->GetTransform()->GetGlobalWorldMatrix();
					renderJob->animationBuffer->mappedData[updateCount].currentTransforms[k] = Math::Matrix4x4::Transpose( finalTransform );
				}
			}
			if ( this->boneAnimation->frameCache[i].size() != 0)
			{
				renderJob->animationBuffer->mappedData[updateCount].currentTransforms[63] = Math::Matrix4x4::Transpose(this->gameObject->GetTransform()->GetLocalWorldMatrix());
			}
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
	this->RegisterBoneAnimation( gameObject->GetComponent<BoneTransformComponent>() );
	auto geometryCount = this->geometry->geometrys.size();
	auto materialCount = this->material->materialIndexs.size();

	if ( materialCount != 1 && geometryCount != materialCount )
	{
		DebugErrorMessage( "Material Count Not Matched Geometry" );
	}
	for ( size_t i = 0; i < geometryCount; i++ )
	{
		auto materialIndex = materialCount == 1 ? 0 : i;
		auto job = KG::Renderer::KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob( this->material->shaders[materialIndex], this->geometry->geometrys[i] );
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

void KG::Component::Render3DComponent::RegisterBoneAnimation( BoneTransformComponent* anim )
{
	this->boneAnimation = anim;
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
	this->geometrys[slotIndex] = inst->LoadGeometry( geometryID, subMeshIndex );
}

bool KG::Component::GeometryComponent::HasBone() const
{
	bool result = false;
	for ( auto* i : geometrys )
	{
		if ( i->HasBone() )
			return true;
	}
	return false;
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
		this->directionalLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "DirectionalLight"_id ) );
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
		this->pointLightShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( Utill::HashString( "MeshVolumeLight"_id ) );
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

void KG::Component::BoneTransformComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );

}

KG::Core::GameObject* KG::Component::BoneTransformComponent::BoneIndexToGameObject( UINT index, UINT submeshIndex ) const
{
	return this->frameCache[submeshIndex][index];
}

void KG::Component::BoneTransformComponent::InitializeBone( KG::Core::GameObject* rootNode )
{
	this->geometry = gameObject->GetComponent<KG::Component::GeometryComponent>();

	this->frameCache.resize( this->geometry->geometrys.size() );
	for ( size_t i = 0; i < this->geometry->geometrys.size(); i++ )
	{
		auto& boneIds = this->geometry->geometrys[i]->boneIds;
		auto& cache = this->frameCache[i];
		cache.resize( boneIds.size() );
		rootNode->MatchBoneToObject( boneIds, cache );
#ifdef _DEBUG
		for ( auto& i : cache )
		{
			if ( i == nullptr )
			{
				DebugErrorMessage( "Bone Not Linked Object!" );
			}
		}
#endif
	}
}

//이퀄레인지 이상함
static float GetTimeData(const std::vector<KG::Utill::KeyData>& data, float currentTime, float duration, float defaultValue = 0.0f)
{
	float value = defaultValue;
	if ( !data.empty() )
	{
		value = data[0].value;
		KG::Utill::KeyData tempData;
		tempData.keyTime = currentTime;
		tempData.value = 0.0f;
		auto p = std::equal_range( data.begin(), data.end(), tempData );
		auto prev = p.first != data.begin() ? std::prev(p.first) : p.first;
		auto last = p.second;

		float keyTime0 = 0.0f;
		float keyTime1 = 0.0f;
		float keyValue0 = 0.0f;
		float keyValue1 = 0.0f;

		if ( prev == data.end() )
		{
			keyTime0 = data.back().keyTime;
			keyValue0 = data.back().value;
		}
		//else if ( prev == data.begin() )
		//{
		//	keyTime0 = data.back().keyTime;
		//	keyValue0 = data.back().value;
		//}
		else 
		{
			keyTime0 = prev->keyTime;
			keyValue0 = prev->value;
		}

		if ( last == data.end() )
		{
			keyTime1 = duration;
			keyValue1 = value;
		}
		else 
		{
			keyTime1 = last->keyTime;
			keyValue1 = last->value;
		}
		value = KG::Math::Lerp( keyValue0, keyValue1, abs( currentTime - keyTime0 ) / abs( keyTime1 - keyTime0 ) );
		//value = keyValue0;
	}
	return value;
}

static std::tuple<XMFLOAT3, XMFLOAT4, XMFLOAT3> GetAnimationTransform( const KG::Utill::NodeAnimation& anim, float currentTime, float duration)
{
	XMFLOAT3 t = {};
	t.x = GetTimeData( anim.translation.x, currentTime, duration );
	t.y = GetTimeData( anim.translation.y, currentTime, duration );
	t.z = GetTimeData( anim.translation.z, currentTime, duration );
	XMFLOAT3 r = {};
	r.x = XMConvertToRadians(GetTimeData( anim.rotation.x, currentTime, duration ));
	r.y = XMConvertToRadians(GetTimeData( anim.rotation.y, currentTime, duration ));
	r.z = XMConvertToRadians(GetTimeData( anim.rotation.z, currentTime, duration ));
	XMFLOAT4 rQuat = KG::Math::Quaternion::Multiply( KG::Math::Quaternion::FromXYZEuler( r ), anim.preRotation );
	XMFLOAT3 s = {};
	s.x = GetTimeData( anim.scale.x, currentTime, duration, 1.0f );
	s.y = GetTimeData( anim.scale.y, currentTime, duration, 1.0f );
	s.z = GetTimeData( anim.scale.z, currentTime, duration, 1.0f );
	return std::make_tuple( t, rQuat, s );
}

void KG::Component::AnimationStreamerComponent::OnCreate( KG::Core::GameObject* gameObject )
{
	IRenderComponent::OnCreate( gameObject );
	this->MatchNode();
}

void KG::Component::AnimationStreamerComponent::MatchNode()
{
	for ( auto& layer : this->anim->layers )
	{
		auto& cache = this->frameCache.emplace_back();
		std::vector<KG::Utill::HashString> ids;
		cache.resize( layer.nodeAnimations.size() );
		for ( auto& n : layer.nodeAnimations )
		{
			ids.push_back( n.nodeId );
		}
		this->gameObject->MatchBoneToObject( ids, cache );
	}
}

void KG::Component::AnimationStreamerComponent::Update( float elapsedTime )
{
	this->timer += elapsedTime * 0.5f;
	if ( this->timer > this->duration ) this->timer -= this->duration;

	//FileLogStreamNone( "--------------------------------------------" );
	//FileLogStreamNone( "Animation One Loop" );
	for ( size_t i = 0; i < this->anim->layers[0].nodeAnimations.size(); i++ )
	{
		if ( this->anim->layers[0].nodeAnimations[i].nodeId == KG::Utill::HashString( "RootNode"_id ) )
		{
			continue;
		}
		//FileLogStreamNone( this->anim->layers[0].nodeAnimations[i].nodeId.srcString );
		auto tuple = GetAnimationTransform( this->anim->layers[0].nodeAnimations[i], this->timer, this->duration );
		auto t = std::get<0>( tuple );
		auto r = std::get<1>( tuple );
		//FileLogStreamNone( "Quat : " << r );
		//FileLogStreamNone( "Eualer : " << KG::Math::Quaternion::ToEuler( r, false ) );
		auto s = std::get<2>( tuple );

		//this->frameCache[0][i]->GetTransform()->SetPosition( t );
		this->frameCache[0][i]->GetTransform()->SetRotation( r );
		this->frameCache[0][i]->GetTransform()->SetScale( s );
	}
	//FileLogStreamNone( "--------------------------------------------" );
}

void KG::Component::AnimationStreamerComponent::InitializeAnimation( const KG::Utill::HashString& animationId, UINT animationIndex )
{
	auto* inst = KG::Resource::ResourceContainer::GetInstance();
	this->anim = inst->LoadAnimation( animationId, animationIndex );
	this->GetDuration();
}

void KG::Component::AnimationStreamerComponent::GetDuration()
{
	for ( auto& a : this->anim->layers[0].nodeAnimations)
	{
		if ( !a.translation.x.empty() ) this->duration = std::max( this->duration, a.translation.x.back().keyTime );
		if ( !a.translation.y.empty() ) this->duration = std::max( this->duration, a.translation.y.back().keyTime );
		if ( !a.translation.z.empty() ) this->duration = std::max( this->duration, a.translation.z.back().keyTime );
		if ( !a.rotation.x.empty() ) this->duration = std::max( this->duration, a.rotation.x.back().keyTime );
		if ( !a.rotation.y.empty() ) this->duration = std::max( this->duration, a.rotation.y.back().keyTime );
		if ( !a.rotation.z.empty() ) this->duration = std::max( this->duration, a.rotation.z.back().keyTime );
		if ( !a.scale.x.empty() ) this->duration = std::max( this->duration, a.scale.x.back().keyTime );
		if ( !a.scale.y.empty() ) this->duration = std::max( this->duration, a.scale.y.back().keyTime );
		if ( !a.scale.z.empty() ) this->duration = std::max( this->duration, a.scale.z.back().keyTime );
	}
}
