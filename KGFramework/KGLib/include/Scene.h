#pragma once
#include "ISerializable.h"
#include "GameObject.h"
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

		ObjectPool objectPool;
		ActivePool frontActivePool; //0������ �����ϴ� ��Ÿ�� ���� ������Ʈ
		ActivePool backActivePool; //UINT_MAX ���� �����ϴ� �̸� ������ ������Ʈ

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetFrontObject( UINT32 index );
		KG::Core::GameObject* GetBackObject( UINT32 index );
		KG::Core::GameObject* FindFrontObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindBackObjectWithTag( const KG::Utill::HashString& tag );
		static UINT32 ToBackID( UINT32 frontID);
	public:
		// ����ִ� ������Ʈ �Ҵ� // ��Ÿ��
		KG::Core::GameObject* CreateNewObject();
		// ����ִ� ������Ʈ �Ҵ� // �̸� ������
		KG::Core::GameObject* CreateNewObject( UINT32 instanceID );
		// ���� ��ü���� �����ؿ�
		//KG::Core::GameObject* CreateNewObject( const KG::Core::GameObject* sourceObject );
		// XML�� ���ǵ��ִ� ���� ��ü���� �����ؿ�
		//KG::Core::GameObject* CreateNewObjcet( const KG::Utill::HashString& sourceObjectId );

		KG::Core::GameObject* FindObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindObjectWithID( UINT32 instanceID );

		// ISerializable��(��) ���� ��ӵ�
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDrawGUI() override;
	};
}
