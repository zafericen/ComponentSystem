#ifndef INDEXER_H
#define	INDEXER_H

#include "RigitArray.h"

namespace CECS
{

	template<typename T>
	class Indexer
	{
	private:
		using EmptyIndexContainer = RigitArray<T>;

		T m_nextIndex;
		EmptyIndexContainer m_emptyIndices;

	public:
		Indexer(T start = 0)
			:m_nextIndex{start}
		{
		}

		T createIndex()
		{
			if (m_emptyIndices.empty())
			{
				return m_nextIndex++;
			}

			T out{ m_emptyIndices.back() };
			m_emptyIndices.popBack();

			return out;
		}

		void releaseIndex(T index)
		{
			m_emptyIndices.pushBack(index);
		}

	};
}

#endif
