#pragma once
#include <string>
#include <vector>
#include "ISerializable.h"
#include "IComponent.h"
#include "ComponentContainer.h"
namespace KG::Component
{
	class TransformComponent;
}
namespace KG::Core
{
	class Scene;
	using KG::Component::ComponentContainer;
	class GameObject : public ISerializable
	{
		bool isDestroy = false;
		ComponentContainer components;
		KG::Core::Scene* ownerScene = nullptr;
		UINT32 instanceID = -1;

		KG::Core::GameObject* InternalFindChildObject(const KG::Utill::HashString& tag) const;
		void InternalMatchBoneToObject(const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones) const;
	public:
		KG::Utill::HashString tag = KG::Utill::HashString(0);

		template <class Ty>
		auto GetComponent() const
		{
			return this->components.GetComponent<Ty>();
		}
		auto GetComponentWithID(const KG::Utill::HashString& componentID) const
		{
			return this->components.GetComponentWithID(componentID);
		};


		template <class Ty>
		auto AddComponent(Ty* cmp)
		{
			cmp->Create(this);
			return this->components.AddComponent<Ty>(cmp);
		}
		void AddComponentWithID(const KG::Utill::HashString& componentID, KG::Component::IComponent* component)
		{
			component->Create(this);
			this->components.AddComponentWithID(componentID, component);
		}

		bool IsDestroy() const;
		void Destroy();

		void SetOwnerScene(KG::Core::Scene* ownerScene);
		void SetInstanceID(UINT32 instanceID);

		KG::Component::TransformComponent* GetTransform() const;
		KG::Core::GameObject* GetChild() const;
		KG::Core::GameObject* GetSibling() const;
		KG::Core::GameObject* FindChildObject(const KG::Utill::HashString& tag) const;
		KG::Core::Scene* GetScene() const;
		UINT32 GetInstanceID() const;
		void MatchBoneToObject(const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones) const;


		virtual void SaveToPrefab(const std::string& name);



		std::vector<std::pair<KG::Utill::HashString, KG::Component::IComponent*>> temporalComponents;

		// ISerializable��(��) ���� ��ӵ�
		virtual void OnDataLoad(tinyxml2::XMLElement* parentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;

	};
}