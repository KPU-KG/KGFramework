#include "pch.h"
#include "fbxImpoter.h"
#include "GameObject.h"
#include "KGGeometry.h"
#include "MathHelper.h"
#include "ResourceContainer.h"

#include "AnimationComponent.h"
#include "GeometryComponent.h"

using namespace DirectX;

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
static float GetTimeData( const std::vector<KG::Utill::KeyData>& data, float currentTime, float duration, float defaultValue = 0.0f )
{
	float value = defaultValue;
	if ( !data.empty() )
	{
		value = data[0].value;
		KG::Utill::KeyData tempData;
		tempData.keyTime = currentTime;
		tempData.value = 0.0f;
		auto p = std::equal_range( data.begin(), data.end(), tempData );
		auto prev = p.first != data.begin() ? std::prev( p.first ) : p.first;
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

static std::tuple<XMFLOAT3, XMFLOAT4, XMFLOAT3> GetAnimationTransform( const KG::Utill::NodeAnimation& anim, float currentTime, float duration )
{
	XMFLOAT3 t = {};
	t.x = GetTimeData( anim.translation.x, currentTime, duration );
	t.y = GetTimeData( anim.translation.y, currentTime, duration );
	t.z = GetTimeData( anim.translation.z, currentTime, duration );
	XMFLOAT3 r = {};
	r.x = XMConvertToRadians( GetTimeData( anim.rotation.x, currentTime, duration ) );
	r.y = XMConvertToRadians( GetTimeData( anim.rotation.y, currentTime, duration ) );
	r.z = XMConvertToRadians( GetTimeData( anim.rotation.z, currentTime, duration ) );
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
	for ( auto& a : this->anim->layers[0].nodeAnimations )
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
