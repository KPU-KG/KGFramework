#pragma once
#include <DirectXMath.h>
#include "MathHelper.h"
#include "IHierarchy.h"
#include "IComponent.h"
#include "Debug.h"
namespace KG::Component
{
	using KG::Core::IHierarchy;
	using namespace DirectX;
	class TransformComponent : public IHierarchy<TransformComponent>, public IComponent
	{
		mutable bool isDirtyGlobal = true; // 부모 상태 변화
		mutable bool isDirtyLocal = true; // 본인 상태 변화
		mutable bool isUseRawMatrix = false;

		mutable XMFLOAT4X4 globalWorldMatrix;
		mutable XMFLOAT4X4 localWorldMatrix;

		XMFLOAT4X4 localPivotTransform;

		XMFLOAT3 position = XMFLOAT3( 0, 0, 0 );
		XMFLOAT4 rotation = XMFLOAT4( 0, 0, 0, 1 ); //사원수
		XMFLOAT3 scale = XMFLOAT3( 1, 1, 1 );

		virtual void OnCreate( KG::Core::GameObject* gameObject )
		{
			//position = XMFLOAT3( 0, 0, 0 );
			//rotation = XMFLOAT4( 0, 0, 0, 1 ); //사원수
			//scale = XMFLOAT3( 1, 1, 1 );
		};
	public:

		XMFLOAT4X4 GetPiviot() const
		{
			return this->localPivotTransform;
		}

		//Position
		XMFLOAT3 GetPosition() const
		{
			return this->position;
		}
		XMFLOAT3 GetWorldPosition() const
		{
			const auto& worldMatrix = this->GetGlobalWorldMatrix();
			return XMFLOAT3( worldMatrix._41 / worldMatrix._44, worldMatrix._42 / worldMatrix._44, worldMatrix._43 / worldMatrix._44 );
		}

		void SetPivot(const XMFLOAT4X4 pivot)
		{
			this->localPivotTransform = pivot;
		}

		void SetPosition( const XMFLOAT3& position )
		{
			this->position = position;
			TurnOnLocalDirtyFlag();
		}
		void XM_CALLCONV SetPosition( const FXMVECTOR& position )
		{
			XMStoreFloat3( &this->position, position );
			TurnOnLocalDirtyFlag();
		}
		void SetPosition( float x, float y, float z )
		{
			this->SetPosition( XMFLOAT3( x, y, z ) );
		}

		//Quaternion
		XMFLOAT4 GetRotation() const
		{
			return this->rotation;
		}
		void XM_CALLCONV SetRotation( const FXMVECTOR& rotation )
		{
			XMStoreFloat4( &this->rotation, rotation );
			TurnOnLocalDirtyFlag();
		}
		void SetRotation( const XMFLOAT4& rotation )
		{
			this->rotation = rotation;
			TurnOnLocalDirtyFlag();
		}
		void SetRotation( float x, float y, float z, float w )
		{
			this->SetRotation( XMFLOAT4( x, y, z, w ) );
		}

		//EulerAngle
		XMFLOAT3 GetEulerAngle() const
		{
			return KG::Math::Quaternion::ToEuler( this->rotation );
		}
		void SetEulerAngle( const XMFLOAT3& angle )
		{
			auto angleQuat = XMQuaternionRotationRollPitchYawFromVector( XMLoadFloat3( &angle ) );
			this->SetRotation( angleQuat );
		}
		void SetEulerAngle( float x, float y, float z )
		{
			this->SetEulerAngle( XMFLOAT3( x, y, z ) );
		}

		//Scale
		XMFLOAT3 GetScale() const
		{
			return this->scale;
		}
		void SetScale( const XMFLOAT3& scale )
		{
			this->scale = scale;
			TurnOnLocalDirtyFlag();
		}
		void SetScale( float x, float y, float z )
		{
			this->SetScale( XMFLOAT3( x, y, z ) );
		}

		void Translate( const XMFLOAT3& move )
		{
			using namespace KG::Math;
			this->SetPosition( Vector3::Add( this->position, move ) );
		}
		void Translate( float x, float y, float z )
		{
			using namespace KG::Math;
			this->SetPosition( Vector3::Add( this->position, XMFLOAT3( x, y, z ) ) );
		}


		void TurnOnLocalDirtyFlag()
		{
			this->isDirtyLocal = true;
			TurnOnGlobalDirtyFlag();
		}
		void TurnOnGlobalDirtyFlag( bool onTree = true )
		{
			this->isDirtyGlobal = true;
			if ( !onTree )
			{
				this->FunctionChild(
					[]( TransformComponent* ptr )
					{
						ptr->TurnOnGlobalDirtyFlag( false );
					}
				);
			}
		}

		void XM_CALLCONV Rotate( const FXMVECTOR& quaternion )
		{
			auto srcQuatVector = XMLoadFloat4( &this->rotation );
			auto result = XMQuaternionMultiply( srcQuatVector, quaternion );
			XMStoreFloat4( &this->rotation, result );
			this->TurnOnLocalDirtyFlag();
		}
		void Rotate( const XMFLOAT4& quaternion )
		{
			this->Rotate( XMLoadFloat4( &quaternion ) );
		}

		void RotateAxis( const XMFLOAT3& axis, float angle )
		{
			this->Rotate( XMQuaternionRotationAxis( XMLoadFloat3( &axis ), XMConvertToRadians( angle ) ) );
		}
		void RotateEuler( const XMFLOAT3& euler )
		{
			this->Rotate( XMQuaternionRotationRollPitchYawFromVector( XMLoadFloat3( &euler ) ) );
		}
		void RotateEuler( float x, float y, float z )
		{
			this->Rotate( XMQuaternionRotationRollPitchYaw( XMConvertToRadians( x ), XMConvertToRadians( y ), XMConvertToRadians( z ) ) );
		}

		void Update()
		{
			this->GetGlobalWorldMatrix();
		}

		const XMFLOAT4X4& GetLocalWorldMatrix() const
		{
			if ( this->isDirtyLocal && !isUseRawMatrix )
			{

				auto pivotMat = XMLoadFloat4x4( &this->localPivotTransform );
				auto rotMat = XMMatrixRotationQuaternion( XMLoadFloat4( &this->rotation ) );
				auto scaleMat = XMMatrixScalingFromVector( XMLoadFloat3( &this->scale ) );
				auto tralationMat = XMMatrixTranslationFromVector( XMLoadFloat3( &this->position ) );

				//최적화 필요
				XMStoreFloat4x4( &this->localWorldMatrix, pivotMat * scaleMat * rotMat * tralationMat );

				this->isDirtyLocal = false;
			}
			return this->localWorldMatrix;
		}
		const XMFLOAT4X4& GetGlobalWorldMatrix() const
		{
			if ( this->isDirtyGlobal )
			{
				if ( this->parent )
				{
					auto parentMatrix = XMLoadFloat4x4( &this->parent->GetGlobalWorldMatrix() );
					auto thisMatrix = XMLoadFloat4x4( &this->GetLocalWorldMatrix() );
					XMStoreFloat4x4( &this->globalWorldMatrix, parentMatrix * thisMatrix );
				}
				else
				{
					auto thisMatrix = XMLoadFloat4x4( &this->GetLocalWorldMatrix() );
					XMStoreFloat4x4( &this->globalWorldMatrix, thisMatrix );
				}
				this->isDirtyGlobal = false;
				//DebugNormalMessage( "GlobalWorldMatrixChange" );
			}
			return this->globalWorldMatrix;
		}

		XMFLOAT3 GetLook() const
		{
			if ( isUseRawMatrix )
			{
				return XMFLOAT3(
					this->localWorldMatrix._31,
					this->localWorldMatrix._32,
					this->localWorldMatrix._33
				);
			}
			else
			{
				auto p = XMLoadFloat3( &KG::Math::look );
				auto q = XMLoadFloat4( &this->rotation );
				auto r = XMVector3Rotate( p, q );
				XMFLOAT3 result;
				XMStoreFloat3( &result, r );
				return result;
			}
		}
		XMFLOAT3 GetUp() const
		{
			if ( isUseRawMatrix )
			{
				return XMFLOAT3(
					this->localWorldMatrix._21,
					this->localWorldMatrix._22,
					this->localWorldMatrix._23
				);
			}
			else
			{
				auto p = XMLoadFloat3( &KG::Math::up );
				auto q = XMLoadFloat4( &this->rotation );
				auto r = XMVector3Rotate( p, q );
				XMFLOAT3 result;
				XMStoreFloat3( &result, r );
				return result;
			}
		}
		XMFLOAT3 GetRight() const
		{
			if ( isUseRawMatrix )
			{
				return XMFLOAT3(
					this->localWorldMatrix._11,
					this->localWorldMatrix._12,
					this->localWorldMatrix._13
				);
			}
			else
			{
				auto p = XMLoadFloat3( &KG::Math::right );
				auto q = XMLoadFloat4( &this->rotation );
				auto r = XMVector3Rotate( p, q );
				XMFLOAT3 result;
				XMStoreFloat3( &result, r );
				return result;
			}
		}
		XMFLOAT3 GlobalTransformNormal( const XMFLOAT3& normal ) const
		{
			auto p = XMLoadFloat3( &normal );
			auto m = XMLoadFloat4x4( &this->GetGlobalWorldMatrix() );
			auto r = XMVector3TransformNormal( p, m );
			XMFLOAT3 result;
			XMStoreFloat3( &result, r );
			return result;
		}
		XMFLOAT3 GetWorldLook() const
		{
			return GlobalTransformNormal( Math::look );
		}
		XMFLOAT3 GetWorldUp() const
		{
			return GlobalTransformNormal( Math::up );
		}
		XMFLOAT3 GetWorldRight() const
		{
			return GlobalTransformNormal( Math::right );
		}

	};
	REGISTER_COMPONENT_ID( KG::Component::TransformComponent );
}