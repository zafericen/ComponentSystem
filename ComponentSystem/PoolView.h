#ifndef POOLVIEW_H
#define	POOLVIEW_H

#include <vector>
#include <tuple>

#include "Core.h"

#include "Accessor.h"
#include "Cluster.h"
#include "ObserverPtr.h"
#include "Signature.h"

namespace CECS
{
	template<typename... Args>
	class PoolView
	{
	private:
		template<typename... Args>
		class PoolIterator
		{
		private:
			ClusterContainer::ClusterWindow& m_clusters;
			std::vector<TypeName>& m_typeNames;
			Index m_clusterIndex;
			Index m_itemIndex;
			Cluster::Cache<Args...> m_cache;

		public:
			PoolIterator(
				ClusterContainer::ClusterWindow& clusters, 
				std::vector<TypeName>& typeNames, 
				Index clusterIndex, 
				Index itemIndex)
				:m_clusters{ clusters }, m_typeNames{typeNames}, m_clusterIndex{ clusterIndex }, m_itemIndex{ itemIndex }
			{
				if (m_clusterIndex < m_clusters.size())
				{
					m_cache = Cluster::Cache<Args...>(*m_clusters[m_clusterIndex],m_typeNames);
				}
			}

			PoolIterator& operator++()
			{
				++m_itemIndex;
				if (m_itemIndex == m_clusters[m_clusterIndex]->size())
				{
					m_itemIndex = 0;
					m_clusterIndex++;
					if (m_clusterIndex < m_clusters.size())
					{
						m_cache = Cluster::Cache<Args...>(*m_clusters[m_clusterIndex],m_typeNames);
					}
				}
				return *this;
			}

			bool operator==(const PoolIterator& right)
			{
				return right.m_clusterIndex == m_clusterIndex && right.m_itemIndex == m_itemIndex;
			}

			bool operator!=(const PoolIterator& right)
			{
				return !(*this == right);
			}

			std::tuple<EntityID, Args&...> operator*()
			{
				return m_cache.getItems(m_itemIndex);
			}
		};

		ClusterContainer::ClusterWindow m_clusters;
		std::vector<TypeName> m_typeNames;

	public:
		PoolView()
		{
			(m_typeNames.push_back(typeid(Args).name()),...);
			setup(Signature{});
		}

		PoolView(const std::vector<TypeName>& needed)
		{
			m_typeNames = needed;
			setup(Signature{});
		}

		PoolView(const std::vector<TypeName>& needed, const std::vector<TypeName>& noNeeded)
		{
			m_typeNames = needed;
			setup(Accessor::createSignature(noNeeded));
		}

		PoolIterator<Args...> begin()
		{
			return PoolIterator<Args...>(m_clusters, m_typeNames, 0, 0);
		}

		PoolIterator<Args...> end()
		{
			return PoolIterator<Args...>(m_clusters, m_typeNames, static_cast<Index>(m_clusters.size()), 0);
		}

	private:
		void setup(const Signature& noIntersection)
		{
			ClusterContainer& container{ Accessor::getClusterContainer() };
			m_clusters = container.getClusters(Accessor::createSignature(m_typeNames), noIntersection);
		}
	};
}

#endif