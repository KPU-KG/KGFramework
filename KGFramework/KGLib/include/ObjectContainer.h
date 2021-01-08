#pragma once
#include <map>
#include <vector>
#include <deque>
#include "GameObject.h"
#include "TransformSystem.h"

namespace KG::Core
{
	class ObjectContainer
	{
	private:
		std::deque<GameObject> pool;
		std::map<std::string, GameObject*> searchPool;
		KG::System::TransformSystem* transformSystem;
	public:
		void PostTransformSystem( KG::System::TransformSystem* system );
		GameObject* CreateNewObject();
		GameObject* GetGameObject(const std::string& name) const;
		void DeleteObject();
	};
}