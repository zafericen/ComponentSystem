#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <map>
#include <bitset>
#include <string>
#include <vector>

#include "Core.h"

namespace CECS
{
	class Signature
	{
	public:
		using Bitset = std::bitset<BITSET_SIZE>;

	private:
		using DynamicBitset = std::map<Index,Bitset>;

		DynamicBitset m_bits;

	public:
		Signature() = default;

		bool check(ComponentID id) const
		{
			return m_bits.at(id/BITSET_SIZE)[id%BITSET_SIZE];
		}

		void set(ComponentID id, bool value = true)
		{
			auto check{ m_bits.find(id/BITSET_SIZE) };

			if (check == m_bits.end()) 
			{
				m_bits[id /BITSET_SIZE] = Bitset{};
			}

			Bitset& bitsetBlock{ m_bits[id/BITSET_SIZE] };
			bitsetBlock.set(id % BITSET_SIZE,value);

			if (!bitsetBlock.any())
			{
				m_bits.erase(id/BITSET_SIZE);
			}
		}

		bool any() const
		{
			return !m_bits.empty();
		}

		Bitset& bitset(Index index)
		{
			return m_bits[index];
		}

		const Bitset& bitset(Index index) const
		{
			return m_bits.at(index);
		}

		bool subset(const Signature& aim) const
		{
			for (auto& pair : m_bits)
			{
				auto check{ aim.m_bits.find(pair.first) };
				if (check == aim.m_bits.end() || 
					(bitset(pair.first) & aim.bitset(pair.first)) != bitset(pair.first))
				{
					return false;
				}
			}
			return true;
		}

		bool anyMatch(const Signature& aim) const
		{
			for (auto& pair : aim.m_bits)
			{
				auto check{ m_bits.find(pair.first) };
				if (check != m_bits.end() &&
					(bitset(pair.first) & aim.bitset(pair.first)).any())
				{
					return true;
				}
			}
			return false;
		}

		bool operator==(const Signature& right) const
		{
			for (auto& pair : right.m_bits)
			{
				auto check{ m_bits.find(pair.first) };
				if (check == m_bits.end() ||
					(bitset(pair.first) == right.bitset(pair.first)))
				{
					return true;
				}
			}
			return false;
		}

		bool operator!=(const Signature& right) const
		{
			return !(*this == right);
		}

		void operator+=(const Signature& signature)
		{
			for (const auto& pair : signature.m_bits)
			{
				auto result{m_bits.find(pair.first)};
				if (result != m_bits.end())
				{
					result->second = result->second | pair.second;
				}
				else
				{
					m_bits[pair.first] = pair.second;
				}
			}
		}

		std::vector<ComponentID> getComponents() const
		{
			std::vector<ComponentID> out;
			for (auto& pair : m_bits)
			{
				for (ComponentID index{}; index < BITSET_SIZE; ++index)
				{
					if (pair.second.test(index))
					{
						out.push_back(pair.first * BITSET_SIZE + index);
					}
				}
			}

			return out;
		}

		const DynamicBitset& getBitsets() const
		{
			return m_bits;
		}
		
	};
}

#endif