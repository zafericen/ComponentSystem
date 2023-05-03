#ifndef CORE_H
#define CORE_H

#include <cstdint>
#include <string>
#include <vector>

namespace CECS
{
	using EntityID = uint32_t;
	using ComponentID = uint32_t;
	using ClusterID = uint16_t;
	using Index = uint32_t;
	using TypeName = std::string;

	inline constexpr EntityID nullent{ 0 };
	inline constexpr ClusterID nullcluster{ 0 };

	inline constexpr size_t BITSET_SIZE{ 64 };
}

#endif