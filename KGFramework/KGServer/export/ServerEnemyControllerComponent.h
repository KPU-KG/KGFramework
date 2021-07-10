#pragma once
#include "ServerBaseComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <unordered_set>

constexpr int MAX_NODE = 5;

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
		eATTACK
	};

	enum class EnemyState {
		eWANDER = 0,			// Ÿ�� ��ġ ���ϱ� -> �̵��ϱ� -> ��� ��� -> Ÿ�� ��ġ ���ϱ�
		eTRACE,					
		eRUNAWAY
	};


	// Action Queue�� �ൿ���� �ְ� ����
	// state���� action�� �ٸ�
	// �� update���� state ��ȯ ���� �˻�
	// �ش� ������Ʈ�� �°� �׼� ���� �˻�

	class SEnemyControllerComponent;



	//////////////////////////////////////////////////////////////////////////////
	// Enemy Actions
	//////////////////////////////////////////////////////////////////////////////

	struct Action {
		SEnemyControllerComponent* enemyComp;
		// ���� �˻�
		// virtual bool CheckCondition() = 0;
		Action(SEnemyControllerComponent* comp) { enemyComp = comp; }
		virtual bool Execute(float elapsedTime) = 0;
		virtual void EndAction() = 0;
	};

	struct IdleAction : public Action {
		IdleAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct SetGoalAction : public Action {
		SetGoalAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct SetTargetAction : public Action {
		SetTargetAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MoveAction : public Action {
		MoveAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct RotateAction : public Action {
		RotateAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct AttackAction : public Action {
		AttackAction(SEnemyControllerComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct State {
		Action* action;
		SEnemyControllerComponent* enemyComp;
		State(SEnemyControllerComponent* comp) { enemyComp = comp; }
		virtual ~State() {}
		virtual void Execute(float elapsedTime) = 0;
		virtual void InitState() = 0;
		virtual float GetValue() = 0;
	};

	struct WanderState : public State {
		const static size_t WANDER_ACTION_COUNT = 4;
		std::array<Action*, WANDER_ACTION_COUNT> action;
		const static int WANDER_ACTION_IDLE = 0;
		const static int WANDER_ACTION_SETGOAL = 1;
		const static int WANDER_ACTION_ROTATE = 2;
		const static int WANDER_ACTION_MOVE = 3;

		int curAction = WANDER_ACTION_IDLE;
		WanderState(SEnemyControllerComponent* comp) : State(comp) {}
		virtual ~WanderState();
		virtual void InitState() override final;

		virtual void Execute(float elapsedTime) override final;

		virtual float GetValue() override final;
	};

	struct TraceState : public State {
		const static int TRACE_ACTION_COUNT = 3;
		std::array<Action*, TRACE_ACTION_COUNT> action;

		const static int TRACE_ACTION_SETGOAL = 0;
		const static int TRACE_ACTION_ROTATE = 1;
		const static int TRACE_ACTION_ATTACK = 2;

		int curAction = TRACE_ACTION_SETGOAL;

		TraceState(SEnemyControllerComponent* comp) : State(comp) {}

		virtual void InitState() override final;

		virtual void Execute(float elapsedTime) override final;

		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct StateManager {
		const static int STATE_WANDER = 0;
		const static int STATE_TRACE = 1;

		const static int STATE_COUNT = 2;
		SEnemyControllerComponent* enemyComp;
		std::array<State*, STATE_COUNT> state;
		int curState = STATE_WANDER;
		StateManager(SEnemyControllerComponent* comp) { enemyComp = comp; }
		~StateManager();

		void Init();

		void SetState();

		void Execute(float elapsedTime);
	};


	static struct MechAnimIndex {
		const static UINT dead = 1U;
		const static UINT shotBigCannon = 6U;
		const static UINT shotSmallCanon = 8U;
		const static UINT walk = 11U;
		const static UINT walkInPlace = 12U;
	};

	class DynamicRigidComponent;
	class IAnimationControllerComponent;
	class SGameManagerComponent;


	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		std::unordered_set<KG::Server::NET_OBJECT_ID> playerId;
		KG::Component::SBaseComponent* target = nullptr;

		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		IAnimationControllerComponent*				anim = nullptr;

		EnemyAction									action = EnemyAction::eIDLE;
		EnemyState									state = EnemyState::eWANDER;

		DirectX::XMFLOAT3							center = { 0,0,0 };			// onCreate���� ������
		float										range = 10;
		DirectX::XMFLOAT3							direction = { 0,0,0 };		// �ϴ� y���� ������� ���� �����̳� ���� ������ ������..?
		DirectX::XMFLOAT3							goal = { 0,0,0 };
		float										speed = 3;

		float										idleInterval = 3;
		float										idleTimer = 0;

		float										rotateInterval = 2;
		float										rotateTimer = 0;
		float										rotateAttackInterval = 0.5;

		float										distance = 0;
		float										arriveTime = 0;
		float										moveTime = 0;

		bool										isDead = false;
		float										destroyInterval = 3.f;
		float										destroyTimer = 0.f;

		float										sendTimer = 0.0f;
		float										sendInterval = 1.f / 60.f;

		bool										changedAnimation = false;

		KG::Component::RaycastCallbackFunc			raycastCallback = nullptr;

        static constexpr int maxHp = 10;
        int											hp = maxHp;

		DirectX::XMFLOAT2							angle;

		DirectX::XMFLOAT3							node[MAX_NODE]{ DirectX::XMFLOAT3(0,0,0), };
		int											nodeCount = 0;
		bool										randomCircuit;
		int											currentNode = 0;

		// �ϴ��� ��ã��� ���� ���� ���� ������ Ÿ�� �÷��̾ ���� ȸ��, �����ϸ� ���ݱ���
		float										traceRange = 10;				

		float										attackInterval = 2;
		float										attackTimer = 0;
		bool										isInAttackDelay = false;
		bool										isAttackable = false;

		StateManager*								stateManager;

	public:

		// move timer ��
		// idle timer ��
		// changed animation ��

		void SetMoveTime(float t) { moveTime = t; }
		void MoveTimer(float elapsedTime) { moveTime += elapsedTime; }
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		void UpdateState();
		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal(float elapsedTime);
		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const float distance) const;
		bool AttackTarget(float elapsedTime);
		// ���� ��Ŷ�� ��� ������ �ұ�
		bool Idle(float elapsedTime);
		bool IsAttackable() const;
		void PostAttack();
		void Attack(SGameManagerComponent* gameManager);
		SEnemyControllerComponent();
		void SetCenter(DirectX::XMFLOAT3 center);
		void SetSpeed(float speed);
		void SetIdleInterval(float interval);
		void SetRotateInterval(float interval);
		void SetWanderRange(float range);
		void SetPosition(DirectX::XMFLOAT3 position);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();
		void SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback);
		void HitBullet();
		bool IsDead() const;
		bool IsDelete() const;
		bool SetTarget();
		bool SetAttackRotation();
		bool IsTargetInRange() const;
		void RegisterPlayerId(KG::Server::NET_OBJECT_ID id);
		void DeregisterPlayerId(KG::Server::NET_OBJECT_ID id);
		KG::Server::NET_OBJECT_ID GetNetId() const { return this->networkObjectId; }
		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);
}