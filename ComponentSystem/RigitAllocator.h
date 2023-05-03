#ifndef RIGITALLOVATOR_H
#define RIGITALLOCATOR_H

#include <memory>

namespace CECS
{
	inline constexpr size_t ARRAY_BYTES{ 500000 };

	template<typename T>
	class RigitAllocator: public std::allocator<T>
	{
	public:
		size_t chunkSize()
		{
			size_t size{ ARRAY_BYTES / sizeof(T) };
			if (size == 0) {
				return 1;
			}
			return size;
		}
	};

}

#endif