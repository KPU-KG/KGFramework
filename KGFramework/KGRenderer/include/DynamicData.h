#pragma once
#include <array>
namespace KG::Resource
{
	struct DynamicBufferReader
	{
		size_t elementSize = 0;
		std::byte* buffer = nullptr;
		template<typename Ty>
		Ty& Get( size_t index, size_t offsetOfByte )
		{
			Ty* ptr = reinterpret_cast<Ty*>(buffer.data() + (elementSize * index) + offsetOfByte);
			return *ptr;
		}
	};
}