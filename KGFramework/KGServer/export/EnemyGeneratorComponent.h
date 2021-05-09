#pragma once
#include "IComponent.h"
#include "ISystem.h"
// #include "ServerBaseComponent.h"
// #include "Debug.h"
// #include "SerializableProperty.h"
// #include "ISerializable.h"
// 
// namespace KG::Component
// {
// 	class SEnemyControllerComponent;
// 	class EnemyGeneratorComponent;
// 	class SGameManagerComponent;
// 
// 	// ���߿� ���� �и��� ��
// 	struct Region {
// 		friend KG::Component::EnemyGeneratorComponent;
// 		friend KG::Component::SGameManagerComponent;
// 
// 		DirectX::XMFLOAT3 position{ 0,0,0 };
// 		float range{ 5 };
// 		Region();
// 		Region(DirectX::XMFLOAT3 position, float range);
// 		Region(const KG::Component::Region& other);
// 		Region(KG::Component::Region&& other);
// 		KG::Component::Region& operator=(const KG::Component::Region& other);
// 		KG::Component::Region& operator=(KG::Component::Region&& other);
// 	private:
// 		KG::Core::SerializableProperty<DirectX::XMFLOAT3> positionProp;
// 		KG::Core::SerializableProperty<float> rangeProp;
// 	};
// 
// 	class DLL EnemyGeneratorComponent : public SBaseComponent {
// 	protected:
// 		std::vector<SEnemyControllerComponent*> enemies;
// 		std::vector<KG::Component::Region> region;
// 		int currentRegion = 0;
// 		bool generateEnemy = false;
// 		void GenerateEnemy();
// 	public:
// 		EnemyGeneratorComponent();
// 		virtual void OnCreate(KG::Core::GameObject* obj) override;
// 		virtual void Update(float elapsedTime) override;
// 
// 		bool IsGeneratable() const;
// 		KG::Component::Region GetNextRegion();
// 		void AddEnemyControllerCompoenent(SEnemyControllerComponent* comp);
// 		int GetCurrentRegionIndex() const;
// 		KG::Component::Region GetCurrentRegion();
// 
// 
// 	public:
// 		void OnDataLoad(tinyxml2::XMLElement* componentElement);
// 		void OnDataSave(tinyxml2::XMLElement* parentElement);
// 		virtual bool OnDrawGUI() override;
// 		bool isChangedProp = false;						// property ������ �ٲ������ true�� �ٲٰ� scene ���� -> �ٽ� false�� �ٲٰ� �ϸ� �ε� ����
// 	};
// 
// 	REGISTER_COMPONENT_ID(EnemyGeneratorComponent);
// }