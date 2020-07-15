#pragma once
#include "hash.h"

namespace KG::Component
{
	template<typename Ty>
	struct ComponentID;

#define REGISTER_COMPONENT_ID(X) template <> struct ComponentID<X> \
		{ static KG::Utill::ID id; } ; KG::Utill::ID ComponentID<X>::id = #X##_id

	class IComponent
	{
	private:
		bool isActive = false;
		bool isDestroy = false;
	public:
		IComponent();
		void SetActive(bool isActive);
		void Destroy();
		bool IsDestroy();

		//Virtual Function
	private:
		virtual void OnCreate() {};
		virtual void OnStart() {};
		virtual void OnEnd() {};
		virtual void OnDestroy() {};
	public:
		virtual void Update(float timeElapsed) {};

	};
	REGISTER_COMPONENT_ID(IComponent);
}