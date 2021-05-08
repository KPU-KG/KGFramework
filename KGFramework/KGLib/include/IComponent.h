#pragma once
#include "hash.h"
#include "ISerializable.h"
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
#define ADD_COMPONENT_ID_TO_ELEMENT(element, Ty) {\
	element->SetAttribute("name", KG::Component::ComponentID<Ty>::name());\
	element->SetAttribute("hash_id", KG::Component::ComponentID<Ty>::id());\
	}\

	struct SystemInformation
	{
		bool isUsing = false;
		bool isReserved = false;
	};

	class IComponent : public KG::Core::ISerializable
	{
	protected:
		KG::Core::GameObject* gameObject = nullptr;
		bool isActive = false;
		SystemInformation systemInfo;
	public:
		IComponent();
		IComponent(const IComponent& other) = delete;
		void SetActive(bool isActive);
		KG::Core::GameObject* GetGameObject() const
		{
			return this->gameObject;
		}
	protected:
		virtual void OnActive()
		{
		};
		virtual void OnDisactive()
		{
		};
		virtual void OnDestroy();
		virtual void OnCreate(KG::Core::GameObject* gameObject)
		{
		};
		virtual void OnStart()
		{
		};
	public:
		void Create(KG::Core::GameObject* gameObject);
		void Reserve(KG::Core::GameObject* gameObject);
		virtual void Update(float timeElapsed);
		virtual void OnDebugUpdate(float timeElasped);
		virtual void Destroy();
		virtual void InternalDestroy();
		virtual void UnReserve();
		void PostUse();
		void PostReserve();
		bool isReserved() const;
		bool isUsing() const;

		// ISerializable을(를) 통해 상속됨
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		virtual bool OnDrawGUI() override;

	};
	REGISTER_COMPONENT_ID(IComponent);
}

namespace ImGui
{
	template<typename Ty>
	bool ComponentHeader()
	{
		return ImGui::CollapsingHeader(KG::Component::template ComponentID<Ty>::name(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);
	}
};

template<typename Ty>
constexpr inline auto GetComponentID()
{
	return KG::Component::ComponentID<Ty>::id();
}
