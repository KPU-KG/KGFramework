#pragma once
#include "hash.h"
namespace KG::Core
{
	class GameObject;
};
namespace KG::Component
{

	template <class Ty> 
	struct ComponentID;

#define REGISTER_COMPONENT_ID(X) template <> struct KG::Component::ComponentID<X> \
		{ inline static constexpr KG::Utill::_ID id() {return #X##_id;}\
		  inline static constexpr const char* name() {return #X;} }

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
		virtual void OnActive() {};
		virtual void OnDisactive() {};
		virtual void OnDestroy();
		virtual void OnCreate( KG::Core::GameObject* gameObject ){};
	public:
		void Create( KG::Core::GameObject* gameObject ) {
			this->gameObject = gameObject;
			PostUse(); 
			this->OnCreate( gameObject );
		}
		virtual void Update(float timeElapsed) {};
		virtual void Destroy() { this->OnDestroy(); };
		void PostUse() { this->systemInfo.isUsing = true; };
		bool isUsing() const { return this->systemInfo.isUsing == true; };

	};
	REGISTER_COMPONENT_ID( IComponent );
}