#pragma once
#include "debug.h"
#include <vector>
namespace KG::Utill
{
	class IndexBackAllocator
	{
		using IndexChecker = std::vector<bool>;
		using index_type = size_t;
	private:
		IndexChecker checker;
		size_t csr = 0;
		size_t emptyCounter = 0;
	public:
		IndexBackAllocator() = default;
		IndexBackAllocator( size_t max )
			: checker( max )
		{
		}
		bool isFull() const
		{
			return this->checker.size() == csr;
		}

		index_type RequestEmptyIndex()
		{
			DebugAssertion( !this->isFull(), L"디스크립터 힙 매니저가 꽉찼습니다." );
			if ( this->emptyCounter == 0 )
			{
				auto result = this->csr;
				csr++;
				return result;
			}
			else
			{
				for ( size_t i = 0; i < csr; i++ )
				{
					if ( this->checker[i] == false )
					{
						this->checker[i] = true;
						this->emptyCounter--;
						return this->csr;
					}
				}
			}
			return -1;
		}
		void Resize( size_t max )
		{
			this->checker.resize( max, false );
		}
		void ReleaseIndex( index_type index )
		{
			DebugAssertion( index < csr, L"디스크립터 힙 매니저가 초과하여 반환되었습니다." );
			this->checker[index] = false;
			if ( index == csr - 1 )
			{
				csr--;
			}
			else
			{
				emptyCounter++;
			}
		}

		bool isEmptyIndex( index_type index )
		{
			return this->checker.at( index );
		}

	};

}