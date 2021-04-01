#pragma once
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "GameObject.h"
#include "ComponentProvider.h"
#include "MaterialMatch.h"
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
		using PresetModelCreator = std::function<std::pair<KG::Utill::HashString, KG::Resource::MaterialMatch>()>;
		using ModelCreator = std::function<KG::Core::GameObject*(const KG::Utill::HashString&, KG::Core::Scene&, const KG::Resource::MaterialMatch&)>;
		using SkyBoxCreator = std::function<void(KG::Core::GameObject&, const KG::Utill::HashString&)>;
		using SkyBoxSetter = std::function<void(const KG::Utill::HashString&)>;
		using GetMatrixFunc = std::function<DirectX::XMFLOAT4X4(KG::Component::IComponent*)>;

		tinyxml2::XMLDocument sourceDocument;
		KG::Component::ComponentProvider* componentProvider = nullptr;
		KG::Component::IComponent* mainCamera = nullptr;

		ObjectPool objectPool;
		ActivePool activePool; //0번부터 시작하는 런타임 생성 오브젝트
		KG::Core::GameObject rootNode;
		
		SceneCameraCreator sceneCameraCreator;
		SkyBoxCreator skyBoxCreator;
		SkyBoxSetter skyBoxSetter;
		ModelCreator modelCreator;
		GetMatrixFunc getViewFunc;
		GetMatrixFunc getProjFunc;

		std::vector<std::string> objectPresetName;
		std::vector<PresetModelCreator> objectPresetModel;
		std::vector<PresetObjectCreator> objectPresetFunc;
		int currentSelectedPreset = 0;

		KG::Utill::HashString skyBoxId = KG::Utill::HashString("SkySnow");

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetIndexObject( UINT32 index ) const;
		UINT32 GetEmptyID();

		//ImGui Variable
		bool isShowHierarchy = true;
		bool isShowGameObjectEdit = true;
		void DrawObjectTree(KG::Core::GameObject* node, KG::Core::GameObject*& focused, int count = 0);
	public:
		Scene();
		bool isStartGame = false;
		void SetComponentProvider(KG::Component::ComponentProvider* componentProvider);
		KG::Component::ComponentProvider* GetComponentProvider() const;
		// 비어있는 오브젝트 할당
		KG::Core::GameObject* CreateNewObject();
		KG::Core::GameObject* CreateNewObject( UINT32 instanceID );
		KG::Core::GameObject* CreateNewTransformObject(); 
		KG::Core::GameObject* CreateNewTransformObject(UINT32 instanceID);
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
		DirectX::XMFLOAT4X4 GetMainCameraView() const;
		DirectX::XMFLOAT4X4 GetMainCameraProj() const;

		void AddSceneCameraObjectCreator(SceneCameraCreator&& creator);
		void AddSkyBoxObjectCreator(SkyBoxCreator&& creator);
		void AddObjectPreset(std::string name, PresetObjectCreator&& creator);
		void AddModelPreset(std::string name, PresetModelCreator&& modelCreator, PresetObjectCreator&& objCreator);
		void AddSkySetter(SkyBoxSetter&& setter);
		void AddModelCreator(ModelCreator&& creator);
		void AddCameraMatrixGetter(GetMatrixFunc&& view, GetMatrixFunc&& proj);
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
