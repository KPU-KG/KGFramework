#pragma once
#include "hash.h"
namespace KG::Core
{
	class GameObject;
};
namespace KG::Component
{
	template<typename Ty>
	struct ComponentID;

#define REGISTER_COMPONENT_ID(X) template <> struct KG::Component::ComponentID<X> \
		{ static KG::Utill::ID id; } ; KG::Utill::ID KG::Component::ComponentID<X>::id = #X##_id

	struct SystemInformation
	{
		bool isUsing = false;
	};

	class IComponent
	{
	protected:
		KG::Core::GameObject* gameObject = nullptr;
		bool isActive = false;
	public:
		SystemInformation systemInfo;

		IComponent();
		void SetActive(bool isActive);
		KG::Core::GameObject* GetGameObject() const
		{
			return this->gameObject;
		}
	protected:
		virtual void OnCreate(KG::Core::GameObject* gameObject) { this->gameObject = gameObject; };
		virtual void OnActive() {};
		virtual void OnDisactive() {};
		virtual void OnDestroy();
	public:
		virtual void Update(float timeElapsed) {};
		void PostUse() { this->systemInfo.isUsing = true; };
		bool isUsing() { return this->systemInfo.isUsing == true; };

	};
}