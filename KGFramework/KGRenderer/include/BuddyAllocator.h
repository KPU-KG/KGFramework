#pragma once
#include <vector>
#include <queue>
namespace KG::Renderer
{

	//요구 조건 : index에 해당하는 정보만 관리 or 블럭을 추상화하여 프레임 리소스로 사용 가능하게 사용

	struct BuddyBlock
	{
		size_t size;
		size_t offset;
	};
	class BuddyAllocator
	{
		static constexpr size_t minBlockSize = 32;
		size_t BlockCount = 64;
		std::queue<BuddyBlock> deleteQueue;
		std::vector<BuddyBlock> freePool;
		BuddyBlock Alloc(size_t reqSize);
	};
	
};