#ifndef CLUSTERCONTAINER_H
#define CLUSTERCONTAINER_H

#include <unordered_map>
#include <set>
#include <memory>
#include <string>

#include "Core.h"

#include "Cluster.h"
#include "Signature.h"
#include "Indexer.h"
#include "ComponentRegister.h"
#include "Cluster.h"
#include "ObserverPtr.h"
#include "Algorithm.h"

namespace CECS
{
	class ClusterGroup
	{
	public:
		using Group = std::set<ClusterID>;

	private:
		Group m_group;

	public:
		ClusterGroup() = default;

		void add(ClusterID id)
		{
			m_group.insert(id);
		}

		void remove(ClusterID id)
		{
			m_group.erase(id);
		}

		bool empty() const
		{
			return m_group.empty();
		}

		ClusterID getDefault() const
		{
			return *m_group.begin();
		}

		const Group& getGroup() const
		{
			return m_group;
		}
	};

	class ClusterContainer
	{
	public:
		using ClusterWindow = std::vector<ObserverPtr<Cluster>>;

	private:
		using Container = std::unordered_map<ComponentID, std::unique_ptr<Cluster>, Hash<ComponentID>>;
		using GroupContainer = std::unordered_map<Signature, ClusterGroup, Hash<Signature>>;

		Container m_clusters;
		GroupContainer m_groups;
		Indexer<ClusterID> m_indexer{1};

	public:
		ClusterContainer() = default;

		~ClusterContainer() = default;

		ObserverPtr<Cluster> createCluster(const Signature& signature, const ComponentRegister& components)
		{
			ClusterID id{ m_indexer.createIndex() };
			std::unique_ptr<Cluster> cluster{ std::make_unique<Cluster>(id,signature) };
			for (ComponentID id : signature.getComponents())
			{
				cluster->addArray(components.getName(id), createArray(components.getRecipie(id)));
			}
			ObserverPtr<Cluster> out{ cluster.get() };
			m_clusters[id] = std::move(cluster);

			if (!m_groups.contains(signature))
			{
				createGroup(signature);
			}

			m_groups[signature].add(id);

			return out;
		}

		void removeCluster(Cluster& cluster)
		{
			ClusterID id{ cluster.getID() };
			ClusterGroup& group{ m_groups[cluster.getSignature()] };
			group.remove(id);
			if (group.empty())
			{
				m_groups.erase(cluster.getSignature());
			}
			m_clusters.erase(id);
			m_indexer.releaseIndex(id);
		}

		ObserverPtr<Cluster> getCluster(ClusterID id)
		{
			return m_clusters[id].get();
		}

		ObserverPtr<Cluster> getCluster(const Signature& signature, const ComponentRegister& components)
		{
			auto result{ m_groups.find(signature) };
			if (result == m_groups.end())
			{
				return createCluster(signature,components);
			}

			return m_clusters[result->second.getDefault()].get();
		}

		ClusterWindow getClusters(const Signature& subset, const Signature& noIntersection)
		{
			ClusterWindow out;
			for (auto& pair : m_groups)
			{
				const Signature& check{ pair.first };

				if (subset.subset(check) && !noIntersection.anyMatch(check))
				{
					for (ClusterID id : pair.second.getGroup())
					{
						out.push_back(getCluster(id));
					}
				}
			}
			return out;
		}

		bool has(ClusterID id) const
		{
			return m_clusters.contains(id);
		}

	private:
		ComponentArrayBase createArray(ObserverPtr<IRecipie> recipie) const
		{
			return recipie->createArray();
		}

		void createGroup(const Signature& signature)
		{
			m_groups[signature] = ClusterGroup{};
		}
	};
}

#endif
