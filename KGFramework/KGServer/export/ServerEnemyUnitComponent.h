#pragma once
#include "ServerBaseComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <unordered_set>

namespace KG::Component
{
	class SEnemyUnitComponent;

	struct Action {
		SEnemyUnitComponent* enemyComp;
		// 조건 검사
		Action(SEnemyUnitComponent* comp) { enemyComp = comp; }
		virtual bool Execute(float elapsedTime) = 0;
		virtual void EndAction() = 0;
	};

	struct State {
		SEnemyUnitComponent* enemyComp;
		State(SEnemyUnitComponent* comp) { enemyComp = comp; }
		virtual ~State() {}
		virtual void Execute(float elapsedTime) = 0;
		virtual void InitState() = 0;
		virtual float GetValue() = 0;
	};

	// struct StateManager {
	// 	SEnemyUnitComponent* enemyComp;
	// 	int curState = 0;
	// 	StateManager(SEnemyUnitComponent* comp);
	// 	virtual ~StateManager();
	// 
	// 	virtual void Init() = 0;
	// 	virtual void SetState() = 0;
	// 	virtual void Execute(float elapsedTime) = 0;
	// 	int GetCurState() const;
	// };

	class DynamicRigidComponent;
	class IAnimationControllerComponent;
	class SGameManagerComponent;


	class DLL SEnemyUnitComponent : public SBaseComponent
	{
	protected:
		std::vector<std::vector<char>> session;
		const char*									presetName;
		std::unordered_set<KG::Server::NET_OBJECT_ID> playerId;
		KG::Component::SBaseComponent*				target = nullptr;

		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		IAnimationControllerComponent*				anim = nullptr;

		DirectX::XMFLOAT3							center = { 0,0,0 };			// onCreate에서 정해줌
		float										range = 10;
		DirectX::XMFLOAT3							direction = { 0,0,0 };		// 일단 y값은 고려하지 않을 예정이나 비행 몹에는 쓸지도..?
		DirectX::XMFLOAT3							goal = { 0,0,0 };
		float										speed = 3;

		float										idleInterval = 3;
		float										idleTimer = 0;
		
		float										rotateInterval = 2;
		float										rotateTimer = 0;
		float										rotateAttackInterval = 0.5;

		bool										isDead = false;
		float										destroyInterval = 3.f;
		float										destroyTimer = 0.f;

		float										sendTimer = 0.0f;
		float										sendInterval = 1.f / 60.f;

		bool										changedAnimation = false;

		KG::Component::RaycastCallbackFunc			raycastCallback = nullptr;

		int											hp;// = maxHp;

		float										attackInterval = 2;
		float										attackTimer = 0;
		bool										isInAttackDelay = false;
		bool										isAttackable = false;
		bool										isAttacked = false;

		bool										isInActive = false;

	public:

		bool IsAttackable() const;
		virtual void PostAttack();
		virtual void Attack(SGameManagerComponent* gameManager);
		void SetSession(std::vector<std::vector<char>> s);
		SEnemyUnitComponent();
		virtual ~SEnemyUnitComponent();
		void SetCenter(DirectX::XMFLOAT3 center);
		void SetSpeed(float speed);
		void SetIdleInterval(float interval);
		void SetRotateInterval(float interval);
		void SetWanderRange(float range);
		void SetPosition(DirectX::XMFLOAT3 position);
		void SetEnemyPresetName(const char* name) { this->presetName = name; }
		const char* GetEnemyPresetName() const { return this->presetName; }
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();
		void SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback);

		virtual void HitBullet();
		bool IsDead() const;
		bool IsDelete() const;
		bool IsAttacked() const;
		virtual void Awake();
		virtual void Sleep();
		
		void RegisterPlayerId(KG::Server::NET_OBJECT_ID id);
		void DeregisterPlayerId(KG::Server::NET_OBJECT_ID id);
		KG::Server::NET_OBJECT_ID GetNetId() const { return this->networkObjectId; }
		virtual void Destroy() override;
		void SetInActive(bool a) { this->isInActive = a; }
		bool GetInActive() const { return this->isInActive; }
	};

	REGISTER_COMPONENT_ID(SEnemyUnitComponent);
}