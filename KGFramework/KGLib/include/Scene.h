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
		ActivePool frontActivePool; //0������ �����ϴ� ��Ÿ�� ���� ������Ʈ
		ActivePool backActivePool; //UINT_MAX ���� �����ϴ� �̸� ������ ������Ʈ
		std::vector<GameObject*> objectTree;

		//GameObject skyBoxObject;
		//GameObject sceneCameraObject;
		
		SceneCameraCreator sceneCameraCreator;
		SkyBoxCreator skyBoxCreator;
		SkyBoxSetter skyBoxSetter;

		std::vector<std::string> objectPresetName;
		std::vector<PresetObjectCreator> objectPresetFunc;
		int currentSelectedPreset = 0;

		KG::Utill::HashString skyBoxId = KG::Utill::HashString("SkySnow");

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetFrontObject( UINT32 index );
		KG::Core::GameObject* GetBackObject( UINT32 index );
		KG::Core::GameObject* FindFrontObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindBackObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* CreateNewBackObject();
		UINT32 GetEmptyBackID();
		size_t GetBackObjectCount() const;
		//back -> front / front -> back
		static UINT32 FlipID(UINT32 frontID);

		//ImGui Variable
		bool isShowHierarchy = true;
		bool isShowGameObjectEdit = true;
	public:
		Scene();
		bool isStartGame = false;
		void SetComponentProvider(KG::Component::ComponentProvider* componentProvider);
		KG::Component::ComponentProvider* GetComponentProvider() const;
		// ����ִ� ������Ʈ �Ҵ�
		KG::Core::GameObject* CreateNewObject(); // ��Ÿ��
		KG::Core::GameObject* CreateNewObject( UINT32 instanceID ); // �̸� ����
		// ���� ��ü���� �����ؿ�
		KG::Core::GameObject* CreateCopyObject( const KG::Core::GameObject* sourceObject ); // ��Ÿ��
		// XML�� ���ǵ��ִ� ���� ��ü���� �����ؿ�
		KG::Core::GameObject* CreatePrefabObjcet( const KG::Utill::HashString& prefabId ); // ��Ÿ��
		KG::Core::GameObject* CreatePrefabObjcet( const KG::Utill::HashString& prefabId, UINT32 instanceID ); // �̸� ����

		KG::Core::GameObject* FindObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindObjectWithID( UINT32 instanceID );

		void LoadScene( const std::string& path );
		void SaveCurrentScene( const std::string& path );

		KG::Component::IComponent* SetMainCamera(KG::Component::IComponent* mainCamera);
		KG::Component::IComponent* GetMainCamera() const;

		void AddSceneCameraObjectCreator(SceneCameraCreator&& creator);
		void AddSkyBoxObjectCreator(SkyBoxCreator&& creator);
		void AddObjectPreset(std::string name, PresetObjectCreator&& creator);
		void AddSkySetter(SkyBoxSetter&& setter);

		// ISerializable��(��) ���� ��ӵ�
	private:
		SerializableProperty<KG::Utill::HashString> skyBoxIdProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual bool OnDrawGUI() override;
	};
}
