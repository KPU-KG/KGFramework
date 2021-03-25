#pragma once
#include "IComponent.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif //PHYSICSPART_EXPORTS

namespace KG::Component
{
	class DLL IPhysicsComponent : public IComponent
	{
	protected:
	public:
	};
};