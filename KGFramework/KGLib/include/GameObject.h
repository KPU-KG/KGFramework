#pragma once
#include <string>
#include "IComponent.h"
#include "ComponentContainer.h"
namespace KG::Core
{
	using KG::Component::ComponentContainer;
	class GameObject
	{
	public:
		std::string name;
		ComponentContainer components;
	};
}