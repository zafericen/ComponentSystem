#ifndef CLUSTERVIEW_H
#define CLUSTERVIEW_H

#include <vector>
#include <tuple>

#include "Core.h"

#include "Cluster.h"
#include "ObserverPtr.h"
#include "Accessor.h"

namespace CECS
{

	template<typename... Args>
	class ClusterView
	{
	private:
		template<typename... Args>
		class ClusterIterator
		{
		private:
			ObserverPtr<Cluster> m_cluster;
			ObserverPtr<std::vector<TypeName>> m_typeNames;
			Cluster::Cache<Args...> m_cache;
			Index m_index;

		public:
			ClusterIterator(ObserverPtr<Cluster> cluster, std::vector<TypeName>& typeNames, Index index)
				:m_cluster{ cluster }, m_typeNames{typeNames}, m_index{ index }
			{
				if (0 < m_cluster.isValid())
				{
					m_cache = Cluster::Cache<Args...>(*m_cluster, *m_typeNames);
				}
			}

			ClusterIterator& operator++()
			{
				++m_index;
				return *this;
			}

			bool operator==(const ClusterIterator& right)
			{
				return m_index == right.m_index;
			}

			bool operator!=(const ClusterIterator& right)
			{
				return m_index != right.m_index;
			}

			std::tuple<EntityID, Args&...> operator*()
			{
				return m_cache.getItems(m_index);
			}

		};

		ObserverPtr<Cluster> m_cluster;
		std::vector<TypeName> m_typeNames;

	public:
		ClusterView(ClusterID id)
		{
			(m_typeNames.push_back(typeid(Args)), ...);
			m_cluster = Accessor::getClusterContainer().getCluster(id);
		}

		ClusterView(ClusterID id, const std::vector<TypeName> typeNames)
		{
			m_typeNames = typeNames;
			m_cluster = Accessor::getClusterContainer().getCluster(id);
		}

		ClusterIterator<Args...> begin()
		{
			return ClusterIterator<Args...>{ m_cluster,m_typeNames,0 };
		}

		ClusterIterator<Args...> end()
		{
			return ClusterIterator<Args...>{ m_cluster,m_typeNames,static_cast<Index>(m_cluster->size()) };
		}
	};
}

#endif