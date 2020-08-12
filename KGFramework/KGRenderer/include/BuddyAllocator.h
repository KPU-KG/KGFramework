#pragma once
#include <vector>
#include <queue>
namespace KG::Renderer
{

	//�䱸 ���� : index�� �ش��ϴ� ������ ���� or ���� �߻�ȭ�Ͽ� ������ ���ҽ��� ��� �����ϰ� ���

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