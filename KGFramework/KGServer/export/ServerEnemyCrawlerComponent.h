#pragma once
#include "ServerBaseComponent.h"
#include "ServerEnemyUnitComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <queue>
#include <array>
#include <unordered_set>

namespace KG::Component
{
	//////////////////////////////////////////////////////////////////////////////
	// Enemy Actions
	//////////////////////////////////////////////////////////////////////////////
	struct CrawlerIdleAction : public Action {
		CrawlerIdleAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerSetTargetAction : public Action {
		CrawlerSetTargetAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerShootAction : public Action {
		CrawlerShootAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerSetAreaAction : public Action {
		CrawlerSetAreaAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerRotateAction : public Action {
		CrawlerRotateAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerChargingAction : public Action {
		CrawlerChargingAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerChargeAttackAction : public Action {
		CrawlerChargeAttackAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerChargeDelayAction : public Action {
		CrawlerChargeDelayAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerShootState : public State {
		const static size_t SHOOT_ACTION_COUNT = 2;
		std::array<Action*, SHOOT_ACTION_COUNT> action;
							
		bool isFinished = false;

		const static size_t SHOOT_ACTION_SET_AREA = 0;
		const static size_t SHOOT_ACTION_ATTACK = 1;

		int curAction = SHOOT_ACTION_SET_AREA;
		CrawlerShootState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerShootState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	struct CrawlerChargeState : public State {
		const static size_t SHOOT_ACTION_COUNT = 4;
		std::array<Action*, SHOOT_ACTION_COUNT> action;

		bool isFinished = false;
		// 타겟은 정해진 상태로 스테이트가 변경된다
		// 그러면 필요한거 : 회전 -> 대기(차징하면서 영역설정) -> 돌진 -> 대기(스턴)

		const static size_t CHARGE_ACTION_ROTATE = 0;
		const static size_t CHARGE_ACTION_CHARGING = 1;
		const static size_t CHARGE_ACTION_ATTACK = 2;
		const static size_t CHARGE_ACTION_DELAY = 3;

		int curAction = CHARGE_ACTION_ROTATE;
		CrawlerChargeState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerChargeState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	struct CrawlerSetTargetState : public State {
		const static size_t SET_TARGET_ACTION_COUNT = 2;
		std::array<Action*, SET_TARGET_ACTION_COUNT> action;

		const static size_t SET_TARGET_ACTION_IDLE = 0;
		const static size_t SET_TARGET_ACTION_SET_TARGET = 1;

		bool isFinished = false;

		int curAction = SET_TARGET_ACTION_IDLE;
		CrawlerSetTargetState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerSetTargetState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerStateManager {
		const static size_t STATE_SET_TARGET = 0;
		const static size_t STATE_SHOOT_ATTACK = 1;
		const static size_t STATE_CHARGE_ATTACK = 2;

		const static size_t STATE_COUNT = 3;

		std::array<State*, STATE_COUNT> state;
		CrawlerStateManager(SEnemyUnitComponent* comp);
		~CrawlerStateManager();

		void Init();
		void SetState();
		void Execute(float elapsedTime);

		SEnemyUnitComponent* enemyComp;
		int curState = 0;
		int GetCurState() const;
	};


	static struct CrawlerAnimIndex {
		const static UINT idle = 0U;
		const static UINT walk = 1U;
	};

	class SCubeAreaRedComponent;

	struct AreaEvent {
		SCubeAreaRedComponent* area = nullptr;
		float timer = 2.6;
	};

	class DLL SEnemyCrawlerComponent : public SEnemyUnitComponent
	{
	protected:
		static constexpr int						maxHp = 100;
		float										areaWidth = 3;
		std::queue<std::pair<float, float>>			shootArea;
		DirectX::XMFLOAT3							shootAreaCenter;
		DirectX::XMFLOAT3							shootTarget;

		float										rotateSpeed = 2;

		float										attackInterval = 0.5;
		float										attackTimer = 0;

		float										chargingTimer = 0;
		float										chargingInterval = 2;
		bool										isCharging = false;

		float										chargeSpeed = 25;
		float										chargeDist = 0;
		float										moveDist = 0;

		float										chargeDelay = 2;
		float										chargeDelayTimer = 0;

		SCubeAreaRedComponent*						area;
		std::vector<AreaEvent>						areaEvent;
		DirectX::XMFLOAT3							chargeTarget;
		DirectX::XMFLOAT3							chargeOrigin;
		DirectX::XMFLOAT3							prevPosition;

		CrawlerStateManager*						stateManager;
		bool										inShootAction = false;
		bool										isFilledArea = false;
	public:
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		virtual void Attack(SGameManagerComponent* gameManager) override;
		SEnemyCrawlerComponent();

		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnDrawGUI();

		virtual void HitBullet() override;

		bool Idle(float elapsedTime);
		bool SetTarget();
		bool SetAttackArea();
		bool Shoot(float elapsedTime);
		bool Rotate(float elapsedTime);
		bool Charging(float elapsedTime);
		bool ChargeAttack(float elapsedTime);
		bool ChargeDelay(float elapsedTIme);

		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyCrawlerComponent);
}