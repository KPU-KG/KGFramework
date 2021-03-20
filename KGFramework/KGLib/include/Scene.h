#pragma once
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "GameObject.h"
#include "ComponentProvider.h"
#include <vector>
#include <deque>
#include <set>
namespace KG::Core
{
	class Scene : public ISerializable
	{
		friend void KG::Core::GameObject::OnDataLoad(tinyxml2::XMLElement* objectElement);
		friend void KG::Core::GameObject::OnPrefabLoad(tinyxml2::XMLElement* objectElement);
		static constexpr UINT NULL_OBJECT = 4294967295;
		using CachePair = std::pair<bool, GameObject>;
		using ObjectPool = std::deque<CachePair>;
		using ActivePool = std::vector<UINT>;
		using SceneCameraCreator = std::function<void (KG::Core::GameObject&)>;
		using PresetObjectCreator = std::function<void(KG::Core::GameObject&)>;
		using SkyBoxCreator = std::function<void(KG::Core::GameObject&, const KG::Utill::HashString&)>;
		using SkyBoxSetter = std::function<void(const KG::Utill::HashString&)>;

		tinyxml2::XMLDocument sourceDocument;
		KG::Component::ComponentProvider* componentProvider = nullptr;
		KG::Component::IComponent* mainCamera = nullptr;

		ObjectPool objectPool;
		ActivePool activePool; //0번부터 시작하는 런타임 생성 오브젝트
		KG::Core::GameObject rootNode;
		
		SceneCameraCreator sceneCameraCreator;
		SkyBoxCreator skyBoxCreator;
		SkyBoxSetter skyBoxSetter;

		std::vector<std::string> objectPresetName;
		std::vector<PresetObjectCreator> objectPresetFunc;
		int currentSelectedPreset = 0;

		KG::Utill::HashString skyBoxId = KG::Utill::HashString("SkySnow");

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetIndexObject( UINT32 index ) const;
		UINT32 GetEmptyID();

		//ImGui Variable
		bool isShowHierarchy = true;
		bool isShowGameObjectEdit = true;
		void DrawObjectTree(KG::Core::GameObject* node, KG::Core::GameObject*& focused, int& count);
	public:
		Scene();
		bool isStartGame = false;
		void SetComponentProvider(KG::Component::ComponentProvider* componentProvider);
		KG::Component::ComponentProvider* GetComponentProvider() const;
		// 비어있는 오브젝트 할당
		KG::Core::GameObject* CreateNewObject(); // 런타임
		KG::Core::GameObject* CreateNewObject( UINT32 instanceID ); // 미리 생성
		// 원본 객체에서 복사해옴
		KG::Core::GameObject* CreateCopyObject( const KG::Core::GameObject* sourceObject ); // 런타임
		// XML에 정의되있는 원본 객체에서 복사해옴
		KG::Core::GameObject* CreatePrefabObjcet( const KG::Utill::HashString& prefabId ); // 런타임
		KG::Core::GameObject* CreatePrefabObjcet( const KG::Utill::HashString& prefabId, UINT32 instanceID ); // 미리 생성

		KG::Core::GameObject* FindObjectWithTag( const KG::Utill::HashString& tag ) const;
		KG::Core::GameObject* FindObjectWithID( UINT32 instanceID ) const;

		UINT GetObjectCount() const;

		void LoadScene( const std::string& path );
		void SaveCurrentScene( const std::string& path );

		KG::Component::IComponent* SetMainCamera(KG::Component::IComponent* mainCamera);
		KG::Component::IComponent* GetMainCamera() const;

		void AddSceneCameraObjectCreator(SceneCameraCreator&& creator);
		void AddSkyBoxObjectCreator(SkyBoxCreator&& creator);
		void AddObjectPreset(std::string name, PresetObjectCreator&& creator);
		void AddSkySetter(SkyBoxSetter&& setter);

		//Root 노드 프리셋으로 초기화 하도록 설정
		void InitializeRoot();
		// ISerializable을(를) 통해 상속됨
	private:
		SerializableProperty<KG::Utill::HashString> skyBoxIdProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual bool OnDrawGUI() override;
	};
}
