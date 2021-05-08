#pragma once
#include <DirectXMath.h>
#include "MathHelper.h"
#include "IHierarchy.h"
#include "IComponent.h"
#include "SerializableProperty.h"
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

		XMFLOAT3 position = XMFLOAT3(0, 0, 0);
		XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 1); //사원수
		XMFLOAT3 scale = XMFLOAT3(1, 1, 1);

		virtual void OnCreate(KG::Core::GameObject* gameObject);
	public:
		TransformComponent();
		//Position
		XMFLOAT3 GetPosition() const;
		XMFLOAT3 GetWorldPosition() const;
		void SetPosition(const XMFLOAT3& position);
		void XM_CALLCONV SetPosition(const FXMVECTOR& position);
		void SetPosition(float x, float y, float z);

		//Quaternion
		XMFLOAT4 GetRotation() const;
		void XM_CALLCONV SetRotation(const FXMVECTOR& rotation);
		void SetRotation(const XMFLOAT4& rotation);
		void SetRotation(float x, float y, float z, float w);

		//EulerAngle
		XMFLOAT3 GetEulerRadian() const;
		void SetEulerRadian(const XMFLOAT3& angle);
		void SetEulerRadian(float x, float y, float z);

		XMFLOAT3 GetEulerDegree() const;
		void SetEulerDegree(const XMFLOAT3& angle);
		void SetEulerDegree(float x, float y, float z);

		//Scale
		XMFLOAT3 GetScale() const;
		void XM_CALLCONV SetScale(const FXMVECTOR& scale);
		void SetScale(const XMFLOAT3& scale);
		void SetScale(float x, float y, float z);

		void Translate(const XMFLOAT3& move);
		void Translate(float x, float y, float z);

		void TurnOnLocalDirtyFlag();
		void TurnOnGlobalDirtyFlag(bool onTree = false);

		void XM_CALLCONV Rotate(const FXMVECTOR& quaternion);
		void Rotate(const XMFLOAT4& quaternion);

		void RotateAxis(const XMFLOAT3& axis, float angle);
		void RotateEuler(const XMFLOAT3& euler);
		void RotateEuler(float x, float y, float z);

		virtual void Update(float elapsedTime) override;

		const XMFLOAT4X4& GetLocalWorldMatrix() const;
		const XMFLOAT4X4& GetGlobalWorldMatrix() const;

		XMFLOAT3 GetLook() const;
		XMFLOAT3 GetUp() const;
		XMFLOAT3 GetRight() const;
		XMFLOAT3 GlobalTransformNormal(const XMFLOAT3& normal) const;
		XMFLOAT3 GetWorldLook() const;
		XMFLOAT3 GetWorldUp() const;
		XMFLOAT3 GetWorldRight() const;
		virtual void OnChangeParent() override;
	private:
		KG::Core::SerializableProperty<XMFLOAT3> positionProp;
		DirectX::XMFLOAT3 eulerRotation;
		DirectX::XMFLOAT3 worldPosition;
		KG::Core::SerializableProperty<XMFLOAT3> worldPositionProp;
		KG::Core::SerializableProperty<XMFLOAT3> rotationEulerProp;
		KG::Core::SerializableProperty<XMFLOAT4> rotationQautProp;
		KG::Core::SerializableProperty<XMFLOAT3> scaleProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;
	};
	REGISTER_COMPONENT_ID(TransformComponent);
}