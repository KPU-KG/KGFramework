#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <vector>

namespace KG::Component
{
	// 1. �־��� ��ġ���� ���� ���� - �̰� GUI �ʿ��ұ�?
	// 2. ���� ������ ���ƴٴϱ� - �̰� ��� ��ġ�ϸ� A*�� ��ã�� �˰��� �����ؾ� �ҵ�

	// action
	// ���
	// �̵�
	// ����
	// �ǰ�

	// state
	// ��ȸ
	// ���� (�÷��̾� �߰�)
	// ���� (?)
	enum class EnemyAction {
		eIDLE = 0,
		eSETGOAL,
		eROTATE,
		eMOVE,
		eATTACK,
		eATTACKED
	};

	enum class EnemyState {
		eWANDER = 0,			// Ÿ�� ��ġ ���ϱ� -> �̵��ϱ� -> ��� ��� -> Ÿ�� ��ġ ���ϱ�
		eTRACE,					
		eRUNAWAY
	};

	class DynamicRigidComponent;
	class AnimationControllerComponent;

	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		AnimationControllerComponent*				anim = nullptr;

		EnemyAction				action = EnemyAction::eSETGOAL;
		EnemyState				state = EnemyState::eWANDER;

		DirectX::XMFLOAT3		center = { 0,0,0 };			// onCreate���� ������
		float					range = 10;
		DirectX::XMFLOAT3		direction = { 0,0,0 };		// �ϴ� z���� ������� ���� �����̳� ���� ������ ������..?
		DirectX::XMFLOAT3		goal = { 0,0,0 };
		float					speed = 3;
		float					idleInterval = 3;
		float					idleTimer = 0;
		float					rotateInterval = 2;
		float					rotateTimer = 0;

		DirectX::XMFLOAT4		quatTo;
		DirectX::XMFLOAT4		quatFrom;
		DirectX::XMFLOAT2		angle;
		void UpdateState();
		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal();
		bool Idle(float elapsedTime);
	public:
		SEnemyControllerComponent();
		void SetCenter(DirectX::XMFLOAT3 center) {
			this->center = center;
		}
		void SetSpeed(float speed) {
			this->speed = speed;
		}
		void SetIdleInterval(float interval) {
			this->idleInterval = interval;
		}
		void SetRotateInterval(float interval) {
			this->rotateInterval = interval;
		}
		void SetWanderRange(float range) {
			this->range = range;
		}
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();

	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);

	class DLL SEnemyControllerComponentSystem : public KG::Component::SBaseComponentSystem<SEnemyControllerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for (auto& com : *this)
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}