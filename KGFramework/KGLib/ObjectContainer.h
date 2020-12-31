#pragma once
#include <map>
#include <vector>
#include <deque>
#include "GameObject.h"

namespace KG::Core
{
	class ObjectContainer
	{
	private:
		std::deque<GameObject> pool;
		std::map<std::string, GameObject*> searchPool;
	public:
		GameObject* CreateNewObject();
		GameObject* GetGameObject(const std::string& name) const;
		void DeleteObject();
	};
}