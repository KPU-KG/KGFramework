// #pragma once
// #include "SerializableProperty.h"
// #include "ServerEnemyControllerComponent.h"
// 
// namespace KG::Core
// {
// 	class GameObject;
// };
// 
// constexpr const int MAX_REGION = 4;
// constexpr const int MAX_ENEMY = 3;
// 
// namespace KG::Component
// {
// 	struct Region {
// 		DirectX::XMFLOAT3 position{ 0,0,0 };
// 		float range{ 5 };
// 	};
// 
// 	// 얘가 에너미 컨트롤러들을 관리
// 
// 	class DLL EnemyGeneratorComponent : public SBaseComponent {
// 	protected:
// 		std::vector<SEnemyControllerComponent*> enemies;
// 		Region region[MAX_REGION];
// 
// 		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
// 	public:
// 		EnemyGeneratorComponent();
// 		virtual void Update(float timeElapsed) override;
// 		virtual bool OnDrawGUI();
// 		void GenerateEnemies(int region);
// 	public:
// 		// KG::Core::SerializableProperty<DirectX::XMFLOAT3> regionPositionProp[MAX_REGION];
// 		// KG::Core::SerializableProperty<float> regionRangeProp[MAX_REGION];
// 	};
// 
// 	REGISTER_COMPONENT_ID(EnemyGeneratorComponent);
// };