#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>
#include <concurrent_unordered_map.h>
#include "SerializableProperty.h"
#include "ISerializable.h"

namespace KG::Physics
{
	class IPhysicsScene;
}

// �� ������
// x 420 z 300
// + ������ 20

namespace KG::Component
{
	class SEnemyUnitComponent;
	class EnemyGeneratorComponent;
	class SGameManagerComponent;

	constexpr int SESSION_TYPE_NONE = 0;
	constexpr int SESSION_TYPE_BUILDING = 1;
	constexpr int SESSION_TYPE_UNIT = 2;

	constexpr unsigned int MAP_SIZE_X = 340;
	constexpr unsigned int MAP_SIZE_Z = 460;

	struct Region {
		friend KG::Component::EnemyGeneratorComponent;
		friend KG::Component::SGameManagerComponent;

		DirectX::XMFLOAT3 position{ 0,0,0 };
		float range{ 5 };
		float heightOffset{ 3 };
		Region();
		Region(DirectX::XMFLOAT3 position, float range, float heightOffset);
		Region(const KG::Component::Region& other);
		Region(KG::Component::Region&& other);
		KG::Component::Region& operator=(const KG::Component::Region& other);
		KG::Component::Region& operator=(KG::Component::Region&& other);
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> positionProp;
		KG::Core::SerializableProperty<float> rangeProp;
		KG::Core::SerializableProperty<float> heightOffsetProp;
	};

	class DLL EnemyGeneratorComponent : public SBaseComponent {
	protected:
		std::vector<SEnemyUnitComponent*> enemies;
		std::vector<KG::Component::Region> region;

		// ���� ������
		std::vector<std::vector<char>> session;

		int currentRegion = 0;
		bool generateEnemy = false;
		bool initialized = false;
		unsigned int score = 0;
		KG::Component::Region GetNextRegion();
		int GetCurrentRegionIndex() const;
		KG::Component::Region GetCurrentRegion();
	public:
		bool isAttackable = false;
		EnemyGeneratorComponent();
		~EnemyGeneratorComponent();
		void Initialize();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		bool IsGeneratable() const;
		void GenerateEnemy();
		void AddEnemyControllerCompoenent(SEnemyUnitComponent* comp);
		void SendAddEnemyPacket(KG::Server::SESSION_ID player);
		unsigned int GetScore() const;
		void RegisterPlayerToEnemy(KG::Server::NET_OBJECT_ID id);
		void DeregisterPlayerToEnemy(KG::Server::NET_OBJECT_ID id);
		void SendAttackPacket(SGameManagerComponent* gameManager);
	public:
		void OnDataLoad(tinyxml2::XMLElement* componentElement);
		void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI() override;
		bool isChangedProp = false;						
		KG::Core::SerializableProperty<bool> generateProp;
	};

	class SPlayerComponent;
	class DLL SGameManagerComponent : public SBaseComponent
	{
		KG::Physics::IPhysicsScene* physicsScene;
		EnemyGeneratorComponent* enemyGenerator;
		void RegisterPlayersToEnemy();
		void UpdatePlayerSession();
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

	public:
	};
	REGISTER_COMPONENT_ID(SGameManagerComponent);
	REGISTER_COMPONENT_ID(EnemyGeneratorComponent);
}