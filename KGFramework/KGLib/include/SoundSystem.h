#pragma once
#include "ISystem.h"
#include "SoundComponent.h"

namespace KG::System 
{
	class SoundSystem : public IComponentSystem<SoundComponent> {
	public:
		virtual void OnUpdate(float elapsedTime) override {
			for (auto& comp : this->pool)
				comp.Update(elapsedTime);
		};
		virtual void OnPostUpdate(float elapsedTime) override {};
		virtual void OnPreRender() override {};
	};
}