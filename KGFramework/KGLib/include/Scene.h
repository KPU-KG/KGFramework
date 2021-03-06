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
		ActivePool frontActivePool; //0번부터 시작하는 런타임 생성 오브젝트
		ActivePool backActivePool; //UINT_MAX 부터 시작하는 미리 생성된 오브젝트

		UINT InternalGetEmptyObject();
		KG::Core::GameObject* GetFrontObject( UINT32 index );
		KG::Core::GameObject* GetBackObject( UINT32 index );
		KG::Core::GameObject* FindFrontObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindBackObjectWithTag( const KG::Utill::HashString& tag );
		static UINT32 ToBackID( UINT32 frontID);
	public:
		// 비어있는 오브젝트 할당 // 런타임
		KG::Core::GameObject* CreateNewObject();
		// 비어있는 오브젝트 할당 // 미리 생성된
		KG::Core::GameObject* CreateNewObject( UINT32 instanceID );
		// 원본 객체에서 복사해옴
		//KG::Core::GameObject* CreateNewObject( const KG::Core::GameObject* sourceObject );
		// XML에 정의되있는 원본 객체에서 복사해옴
		//KG::Core::GameObject* CreateNewObjcet( const KG::Utill::HashString& sourceObjectId );

		KG::Core::GameObject* FindObjectWithTag( const KG::Utill::HashString& tag );
		KG::Core::GameObject* FindObjectWithID( UINT32 instanceID );

		// ISerializable을(를) 통해 상속됨
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDrawGUI() override;
	};
}
