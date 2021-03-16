#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>
namespace KG::Component
{
	class SceneCameraComponent : public IComponent
	{
	private:
		using UpdateFunctor = std::function<void(KG::Core::GameObject*, float)>;
		UpdateFunctor updateFunctor;
	public:
		virtual void Update(float elapsedTime) override;
		virtual void PostUpdateFunction(const UpdateFunctor& functor);
		virtual void OnDestroy() override;

	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
		virtual bool OnDrawGUI() override;
	};
	REGISTER_COMPONENT_ID(SceneCameraComponent);

	class SceneCameraComponentSystem : public KG::System::IComponentSystem<SceneCameraComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override
		{

		}
		virtual void OnPreRender() override
		{

		}

	};
}