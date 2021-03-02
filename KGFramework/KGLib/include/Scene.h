#pragma once
#include "GameObject.h"
#include <vector>
#include <deque>
#include <set>
namespace KG::Core
{
	class Scene
	{
		using ObjectPool = std::deque<std::pair<bool, GameObject>>;

		std::deque<std::pair<bool, GameObject>> objectPool;
		std::vector<std::pair<GameObject*, ObjectPool::iterator>> activePool;

		GameObject* CreateNewObject( const KG::Utill::HashString& id );
		void DeleteObject( const KG::Utill::HashString& id );
		void FindObject( const KG::Utill::HashString& id );
		void ClearObject();
	};
}
