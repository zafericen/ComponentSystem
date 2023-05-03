#ifndef POOL_H
#define POOL_H

#include "Core.h"

#include "ClusterContainer.h"
#include "EntityContainer.h"
#include "ClusterTraits.h"
#include "Indexer.h"

namespace CECS
{
	class Pool
	{
	private:
		ComponentRegister m_register;
		ClusterContainer m_clusters;
		EntityContainer m_entites;
		Indexer<EntityID> m_indexer{1};

		friend class Accessor;

	public:
		Pool() = default;

		~Pool() = default;

		EntityID createEntity()
		{
			EntityID out{ m_indexer.createIndex() };
			m_entites.addEntity(out);
			return out;
		}

		void destroyEntity(EntityID id)
		{
			if (id == nullent)
			{
				return;
			}
			ObserverPtr<Cluster> cluster{ m_entites.getEntityCluster(id) };
			m_entites.removeEntity(id);
			m_indexer.releaseIndex(id);

			if (cluster.isValid())
			{
				cluster->removeEntity(id);
			}
		}

		template<typename T>
		void addComponent(EntityID id, T&& component, const TypeName& name)
		{
			m_register.registerComponent<T>(name);

			ObserverPtr<Cluster>& oldCluster{ m_entites.getEntityCluster(id) };

			Signature signature;
			if (oldCluster.isValid())
			{
				signature = oldCluster->getSignature();
			}
			signature.set(m_register.getID(name));
			ObserverPtr<Cluster> newCluster{ m_clusters.getCluster(signature,m_register) };

			if(oldCluster.isValid())
			{
				ClusterTraits::carryEntity(id, id, *oldCluster, *newCluster);
				checkCluster(*oldCluster);
			}
			else
			{
				newCluster->addEntity(id);
			}
			oldCluster = newCluster;
			newCluster->addComponent(std::move(component), id, name);
		}

		void removeComponent(EntityID id, const TypeName& name)
		{
			ObserverPtr<Cluster>& oldCluster{ m_entites.getEntityCluster(id) };
			Signature signature{ oldCluster->getSignature() };
			signature.set(m_register.getID(name),false);

			if (!signature.any())
			{
				destroyEntity(id);
			}
			else 
			{
				ObserverPtr<Cluster> newCluster{ m_clusters.getCluster(signature,m_register) };
				ClusterTraits::carryEntity(id, id, *oldCluster, *newCluster);
				checkCluster(*oldCluster);
				oldCluster = newCluster;
			}
		}

		template<typename T>
		T& getComponent(EntityID id, const TypeName& name)
		{
			return getCluster(id).getComponent<T>(id, name);
		}

		template<typename T>
		void setComponent(EntityID id, T&& component, const TypeName& name)
		{
			getCluster(id).setComponent<T>(std::move(component), id, name);
		}

		template<typename... Args>
		void addComponents(EntityID id, Args&&... components, const std::vector<TypeName>& names)
		{
			Signature addition{ createSignature<Args...>(names) };
			ObserverPtr<Cluster>& oldCluster{ m_entites.getEntityCluster(id) };

			Signature signature;
			if (oldCluster.isValid())
			{
				signature = oldCluster->getSignature();
			}
			signature += addition;

			ObserverPtr<Cluster> newCluster{ m_clusters.getCluster(signature,m_register) };
			if (oldCluster.isValid())
			{
				ClusterTraits::carryEntity(id, id, *oldCluster, *newCluster);
				checkCluster(*oldCluster);
			}
			else
			{
				newCluster->addEntity(id);
			}
			oldCluster = newCluster;
			newCluster->addComponents<Args...>(std::move(components)..., id, names);
		}

		template <typename... Args>
		Signature createSignature(const std::vector<TypeName>& names)
		{
			Signature out;
			Index index{ 0 };
			((m_register.registerComponent<Args>(names[index]),out.set(m_register.getID(names[index])), ++index), ...);
			return out;
		}

		Cluster& getCluster(EntityID id)
		{
			return *m_clusters.getCluster(m_entites.getClusterID(id));
		}

		void copyEntity(EntityID newEntity, EntityID oldEntity)
		{
			ObserverPtr<Cluster>& currentCluster{ m_entites.getEntityCluster(newEntity) };
			ObserverPtr<Cluster> destinationCluster{ m_entites.getEntityCluster(oldEntity) };

			if (currentCluster.isValid())
			{
				currentCluster->removeEntity(newEntity);
			}
			if (destinationCluster.isValid())
			{
				ClusterTraits::copyEntity(newEntity, oldEntity, *currentCluster, *destinationCluster);
			}
			currentCluster = destinationCluster;
		}

		void carryEntity(EntityID id, ClusterID newCluster)
		{
			ObserverPtr<Cluster>& oldCluster{ m_entites.getEntityCluster(id) };
			ObserverPtr<Cluster> destination{ m_clusters.getCluster(newCluster) };
			ClusterTraits::carryEntity(id, id, *oldCluster, *destination);
			checkCluster(*oldCluster);
			oldCluster = destination;
		}

		bool hasComponent(EntityID id, const TypeName& name)
		{
			return getCluster(id).getSignature().check(m_register.getID(name));
		}

	private:
		void checkCluster(Cluster& cluster)
		{
			if (cluster.empty())
			{
				m_clusters.removeCluster(cluster);
			}
		}
	};
}

#endif