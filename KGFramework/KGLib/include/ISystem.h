#pragma once
#include "IComponent.h"
#include "ComponentProvider.h"
#include <deque>
#include <algorithm>
#include <type_traits>
#include <iterator>

namespace KG::System
{
	using namespace KG::Component;

	template<class Ty>
	class ComponentPooler;

	template <class Ty>
	class UsingComponentIterator
	{
		std::deque<Ty>* srcPool = nullptr;
		class std::deque<Ty>::iterator mainIterator;
	public:
		using ThisType = UsingComponentIterator<Ty>;
		UsingComponentIterator( std::deque<Ty>* srcPool )
			:srcPool( srcPool )
		{
			mainIterator = this->srcPool->begin();
			while ( mainIterator != srcPool->end() )
			{
				if ( mainIterator->isUsing() || mainIterator == srcPool->end() )
				{
					break;
				}
				++mainIterator;
			}
		}

		UsingComponentIterator( std::deque<Ty>* srcPool, class std::deque<Ty>::iterator iter )
			:srcPool( srcPool )
		{
			mainIterator = iter;
		}
		UsingComponentIterator( const ThisType& other ) = default;
		~UsingComponentIterator() = default;
		ThisType& operator=( const ThisType& other ) = default;
		ThisType& operator++()
		{
			do
			{
				++mainIterator;
				if ( mainIterator == srcPool->end() || mainIterator->isUsing())
				{
					break;
				}
			} while ( mainIterator != srcPool->end() );
			return *this;
		}
		Ty& operator*() const
		{
			return *this->mainIterator;
		}
		ThisType operator++( int ) const
		{
			ThisType copy = ThisType( *this );
			do
			{
				++mainIterator;
				if ( mainIterator == srcPool->end() || mainIterator->isUsing() )
				{
					break;
				}
			} while ( mainIterator != srcPool->end() );
			return copy;
		}
		Ty* operator->() const
		{
			return &*this->mainIterator;
		}
		bool operator==( const ThisType& other ) const
		{
			return this->mainIterator == other.mainIterator;
		}
		bool operator!=( const ThisType& other ) const
		{
			return this->mainIterator != other.mainIterator;
		}
	};

	template<class Ty>
	class ComponentPooler
	{
		std::deque<Ty> componentPool;
	public:
		//void Reserve( size_t size )
		//{
		//	this->componentPool.resize( size );
		//}
		Ty* GetNewComponent()
		{
			auto result = std::find_if(
				componentPool.begin(), componentPool.end(),
				[]( Ty& a ) { return !a.isReserved(); }
			);
			if ( result == componentPool.end() )
			{
				return &this->componentPool.emplace_back();
			}
			else
			{
				return &*result;
			}
		}
		UsingComponentIterator<Ty> begin()
		{
			return UsingComponentIterator<Ty>( &this->componentPool );
		}

		UsingComponentIterator<Ty> end()
		{
			return UsingComponentIterator<Ty>( &this->componentPool, this->componentPool.end() );
		}
	};





	class ISystem
	{
	protected:
	public:
		virtual IComponent* GetNewComponent() = 0;
		virtual void OnUpdate(float elapsedTime) = 0;
		virtual void OnDebugUpdate(float elapsedTime)
		{
		};
		virtual void OnPostUpdate( float elapsedTime ) = 0;
		virtual void OnPreRender() = 0;
		virtual void OnPostProvider(KG::Component::ComponentProvider& provider) = 0;
	};

	template <class Ty>
	class IComponentSystem : ISystem
	{
	protected:
		ComponentPooler<Ty> pool;
		virtual void OnGetNewComponent( Ty* ty ) {}
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : this->pool )
			{
				com.Update(elapsedTime);
			}
		}
		virtual void OnPostUpdate(float elapsedTime) override
		{
		}
		virtual Ty* GetNewComponent()
		{
			auto* target = this->pool.GetNewComponent();
			this->OnGetNewComponent( target );
			return target;
		}
		virtual void OnPostProvider(KG::Component::ComponentProvider& provider)
		{
			provider.PostInjectionFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()), 
				[this]( KG::Core::GameObject* object ) 
				{
					auto* comp = this->GetNewComponent();
					object->AddComponent<Ty>(comp);
					return comp;
				}
			);
			provider.PostGetterFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()),
				[this]()->KG::Component::IComponent*
				{
					return static_cast<KG::Component::IComponent*>(this->GetNewComponent());
				}
			);

		}
		UsingComponentIterator<Ty> begin()
		{
			return this->pool.begin();
		}
		UsingComponentIterator<Ty> end()
		{
			return this->pool.end();
		}
		virtual void Clear() 
		{
			for ( auto& comp : pool )
			{
				comp.Destroy();
			}
		};
	};
};
