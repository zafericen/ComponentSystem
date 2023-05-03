#ifndef RIGITARRAY_H
#define RIGITARRAY_H

#include <algorithm>
#include <memory>
#include <vector>
#include <type_traits>

#include "RigitAllocator.h"

namespace CECS
{

	template<typename T>
	class RigitIterator
	{
	private:
		using MultiArray = T**;

		MultiArray m_arrays;
		size_t m_arraySize;
		size_t m_itemIndex;

	public:
		RigitIterator(MultiArray arrays, size_t arraySize, size_t itemIndex)
			:m_arrays{ arrays }, m_arraySize{ arraySize }, m_itemIndex{ itemIndex }
		{
		}

		T& operator*()
		{
			return m_arrays[m_itemIndex / m_arraySize][m_itemIndex % m_arraySize];
		}

		const T& operator*() const
		{
			return m_arrays[m_itemIndex / m_arraySize][m_itemIndex % m_arraySize];
		}

		RigitIterator& operator++()
		{
			++m_itemIndex;
			return *this;
		}

		bool operator==(const RigitIterator& right) const
		{
			return m_itemIndex == right.m_itemIndex;
		}

		bool operator!=(const RigitIterator& right) const
		{
			return m_itemIndex != right.m_itemIndex;
		}
	};

	template<typename T, typename Allocator = RigitAllocator<T>>
	class RigitArray
	{
	private:
		using Array = T*;
		using ArrayContainer = std::vector<Array>;

		ArrayContainer m_arrays;
		Allocator m_allocator;
		size_t m_arraySize;
		size_t m_itemCount{};

	public:
		RigitArray()
			:m_arraySize{ m_allocator.chunkSize() }
		{
		}

		RigitArray(const RigitArray& rigitArray) : RigitArray(std::move(rigitArray.copy()))
		{
		}

		RigitArray(RigitArray&& rigitArray) noexcept :
			m_arrays{ std::move(rigitArray.m_arrays) },
			m_allocator{ std::move(rigitArray.m_allocator) },
			m_arraySize{ rigitArray.m_arraySize },
			m_itemCount{ rigitArray.m_itemCount }
		{
			rigitArray.m_itemCount = 0;
		}

		RigitArray& operator=(const RigitArray& rigitArray)
		{
			clear();
			*this = std::move(rigitArray.copy());
			return *this;
		}

		RigitArray& operator=(RigitArray&& rigitArray) noexcept
		{
			clear();
			m_arrays = std::move(rigitArray.m_arrays);
			m_allocator = std::move(rigitArray.m_allocator);
			m_arraySize = rigitArray.m_arraySize;
			m_itemCount = rigitArray.m_itemCount;
			rigitArray.m_itemCount = 0;
			return *this;
		}

		virtual ~RigitArray()
		{
			clear();
		}

		void pushBack(const T& item)
		{
			pushBack(T{ item });
		}

		void pushBack(T&& item)
		{
			increaseCapacity(m_itemCount + 1);
			constuct(m_itemCount, std::move(item));
			++m_itemCount;
		}

		void popBack()
		{
			destroy(m_itemCount - 1);
			--m_itemCount;
			decreaseCapacity(m_itemCount);
		}

		T& get(size_t index)
		{
			return m_arrays[index / m_arraySize][index % m_arraySize];
		}

		const T& get(size_t index) const
		{
			return m_arrays[index / m_arraySize][index % m_arraySize];
		}

		void set(size_t index, const T& item)
		{
			get(index) = item;
		}

		void set(size_t index, T&& item)
		{
			get(index) = std::move(item);
		}

		T& operator[](size_t index)
		{
			return get(index);
		}

		const T& operator[](size_t index) const
		{
			return get(index);
		}

		T& back()
		{
			return get(m_itemCount - 1);
		}

		const T& back() const
		{
			return get(m_itemCount - 1);
		}

		void swap(size_t left, size_t right)
		{
			std::swap(get(left), get(right));
		}

		void clear()
		{
			for (size_t index{}; index < m_itemCount; ++index)
			{
				destroy(index);
			}
			decreaseCapacity(0);
			m_itemCount = 0;
		}

		size_t size() const
		{
			return m_itemCount;
		}

		size_t capacity() const
		{
			return m_arraySize * m_arrays.size();
		}

		bool empty() const
		{
			return !static_cast<bool>(m_itemCount);
		}

		RigitIterator<T> begin()
		{
			return RigitIterator<T>{m_arrays.data(), m_arraySize, 0};
		}

		RigitIterator<T> end()
		{
			return RigitIterator<T>{nullptr, 0, m_itemCount};
		}

		RigitArray copy() const
		{
			RigitArray<T, Allocator> out;
			for (size_t index{}; index < m_itemCount; ++index)
			{
				out.pushBack(get(index));
			}
			return out;
		}

	private:
		void constuct(size_t index, T&& item)
		{
			std::allocator_traits<Allocator>::construct(m_allocator, &get(index), std::move(item));
		}

		void destroy(size_t index)
		{
			std::allocator_traits<Allocator>::destroy(m_allocator, &get(index));
		}

		void increaseCapacity(size_t newCapacity)
		{
			while (newCapacity > capacity())
			{
				m_arrays.push_back(std::allocator_traits<Allocator>::allocate(m_allocator, m_arraySize));
			}
		}

		void decreaseCapacity(size_t newCapacity)
		{
			if (newCapacity)
			{
				newCapacity += m_arraySize;
			}

			while (capacity() > newCapacity)
			{
				Array removal{ m_arrays.back() };
				std::allocator_traits<Allocator>::deallocate(m_allocator, removal, m_arraySize);
				m_arrays.pop_back();
			}
		}
	};

}

#endif