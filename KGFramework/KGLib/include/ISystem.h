#pragma once
#include "IComponent.h"
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
				if ( mainIterator == srcPool->end() || mainIterator->isUsing() )
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
		ThisType operator++( int )
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
			return *this;
		}
		Ty* operator->() const
		{
			return &*this->mainIterator;
		}
		bool operator==( const ThisType& other )
		{
			return this->mainIterator == other.mainIterator;
		}
		bool operator!=( const ThisType& other )
		{
			return this->mainIterator != other.mainIterator;
		}
	};

	template<class Ty>
	class ComponentPooler
	{
		std::deque<Ty> componentPool;
	public:
		void Reserve( size_t size )
		{
			this->componentPool.resize( size );
		}
		Ty* GetNewComponent()
		{
			auto result = std::find_if(
				componentPool.begin(), componentPool.end(),
				[]( Ty& a ) { return a.isUsing(); }
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
		virtual void OnUpdate( float elapsedTime ) = 0;
		virtual void OnPostUpdate( float elapsedTime ) = 0;
		virtual void OnPreRender() = 0;
	};

	template <class Ty>
	class IComponentSystem : ISystem
	{
	protected:
		ComponentPooler<Ty> pool;
		virtual void OnGetNewComponent( Ty* ty ) {}
	public:
		virtual IComponent* GetNewComponent()
		{
			auto* target = this->pool.GetNewComponent();
			this->OnGetNewComponent( target );
			return target;
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
