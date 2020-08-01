#pragma once
#include "IComponent.h"
#include <deque>
#include <algorithm>
namespace KG::System
{
	using namespace KG::Component;

	template<class Ty>
	class ComponentPooler
	{
		std::deque<Ty> componentPool;
	public:
		void Reserve(size_t size)
		{
			this->componentPool.resize(size);
		}
		Ty* GetNewComponent()
		{
			auto result = std::find_if(
				componentPool.begin(), componentPool.end(),
				[](Ty& a) { return a.isUsing(); }
			);
			if (result == componentPool.end())
			{
				this->componentPool.emplace_back();
			}
			else 
			{
				return &*result;
			}
		}
	};

	class ISystem
	{
	protected:
	public:
		virtual IComponent* GetNewComponent() = 0;
		virtual void OnUpdate(float elapsedTime) = 0;
		virtual void OnPostUpdate(float elapsedTime) = 0;
		virtual void OnPreRender() = 0;
	};

	template <class Ty>
	class IComponentSystem : ISystem
	{
	protected:
		ComponentPooler<Ty> pool;
	public:
		virtual IComponent* GetNewComponent()
		{
			return this->pool.GetNewComponent();
		}
	};
};
