#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>

constexpr const int MAX_NODE = 5;

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

	static struct MechAnimIndex {
		const static UINT dead = 1U;
		const static UINT shotSmallCanon = 8U;
		const static UINT walk = 11U;
		const static UINT walkInPlace = 12U;
	};

	class DynamicRigidComponent;
	class AnimationControllerComponent;


	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		AnimationControllerComponent*				anim = nullptr;

		EnemyAction									action = EnemyAction::eSETGOAL;
		EnemyState									state = EnemyState::eWANDER;

		DirectX::XMFLOAT3							center = { 0,0,0 };			// onCreate���� ������
		float										range = 10;
		DirectX::XMFLOAT3							direction = { 0,0,0 };		// �ϴ� z���� ������� ���� �����̳� ���� ������ ������..?
		DirectX::XMFLOAT3							goal = { 0,0,0 };
		float										speed = 3;
		float										idleInterval = 3;
		float										idleTimer = 0;
		float										rotateInterval = 2;
		float										rotateTimer = 0;

		bool										isDead = false;
		
		KG::Component::RaycastCallbackFunc			raycastCallback = nullptr;

		int											hp = 10;

		DirectX::XMFLOAT2							angle;

		DirectX::XMFLOAT3							node[MAX_NODE]{ DirectX::XMFLOAT3(0,0,0), };
		// std::vector<DirectX::XMFLOAT3>				node;
		int											nodeCount = 0;
		bool										randomCircuit;
		int											currentNode = 0;

		void UpdateState();
		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal();
		bool Idle(float elapsedTime);
		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
	public:
		SEnemyControllerComponent();
		void SetCenter(DirectX::XMFLOAT3 center);
		void SetSpeed(float speed);
		void SetIdleInterval(float interval);
		void SetRotateInterval(float interval);
		void SetWanderRange(float range);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		void SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback);
		void HitBullet();

		std::vector< KG::Core::SerializableProperty<DirectX::XMFLOAT3>> nodeProp;
	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);


}