#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>
#include <concurrent_unordered_map.h>

namespace KG::Physics
{
	class IPhysicsScene;
}

constexpr const int MAX_REGION = 4;
constexpr const int MAX_ENEMY = 3;

namespace KG::Component
{
	class SEnemyControllerComponent;
	struct Region {
		DirectX::XMFLOAT3 position{ 0,0,0 };
		float range{ 5 };
	};

	// 나중에 파일 분리할 것

	class EnemyGenerator {
	protected:
		std::vector<SEnemyControllerComponent*> enemies;
		KG::Component::Region region[MAX_REGION];
		int currentRegion = 0;
	public:
		bool OnDrawGUI(KG::Core::GameObject* gameObject);
		bool IsGeneratable() const;
		KG::Component::Region GetNextRegion();
		void AddEnemyControllerCompoenent(SEnemyControllerComponent* comp);
		int GetCurrentRegionIndex() const;
		KG::Component::Region GetCurrentRegion();
	};

	class SPlayerComponent;
	class EnemyGeneratorComponent;
	class DLL SGameManagerComponent : public SBaseComponent
	{
		KG::Physics::IPhysicsScene* physicsScene;
		EnemyGenerator enemyGenerator;
		bool generateEnemy = false;
		void GenerateEnemy();
	public:
		float updatetimer = 0;
		concurrency::concurrent_unordered_map<KG::Server::NET_OBJECT_ID, KG::Component::SPlayerComponent*> playerObjects;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
		virtual void SetPhysicsScene(KG::Physics::IPhysicsScene* physicsScene) { this->physicsScene = physicsScene; }
		virtual KG::Physics::IPhysicsScene* GetPhysicsScene() { return this->physicsScene; }
	};
	REGISTER_COMPONENT_ID(SGameManagerComponent);

	

}