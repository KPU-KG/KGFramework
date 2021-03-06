#pragma once
#include "ISerializable.h"
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

		tinyxml2::XMLDocument sourceDocument;
		KG::Component::ComponentProvider* componentProvider = nullptr;
		ObjectPool objectPool;
		ActivePool frontActivePool; //0������ �����ϴ� ��Ÿ�� ���� ������Ʈ
		ActivePool backActivePool; //UINT_MAX ���� �����ϴ� �̸� ������ ������Ʈ

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetFrontObject( UINT32 index );
		KG::Core::GameObject* GetBackObject( UINT32 index );
		KG::Core::GameObject* FindFrontObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindBackObjectWithTag( const KG::Utill::HashString& tag );
		//back -> front / front -> back
		static UINT32 FlipID(UINT32 frontID);
	public:
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

		// ISerializable��(��) ���� ��ӵ�
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDrawGUI() override;
	};
}
