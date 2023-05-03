#ifndef ENTITYCONTAINER_H
#define	ENTITYCONTAINER_H

#include "Core.h"

#include "Signature.h"
#include "RigitArray.h"

namespace CECS
{
	class EntityContainer
	{
	private:
		using EntityClusterMap = RigitArray<ObserverPtr<Cluster>>;

		EntityClusterMap m_entites;

	public:
		EntityContainer()
		{
			m_entites.pushBack(nullptr);
		}

		~EntityContainer() = default;

		void addEntity(EntityID id, ObserverPtr<Cluster> cluster= nullptr)
		{
			if (id < m_entites.size())
			{
				m_entites[id] = cluster;
			}
			else 
			{
				m_entites.pushBack(cluster);
			}
		}

		void removeEntity(EntityID id)
		{
			m_entites[id] = nullptr;
		}

		ObserverPtr<Cluster>& getEntityCluster(EntityID id)
		{
			return m_entites[id];
		}

		const Signature& getSignature(EntityID id)
		{
			return m_entites[id]->getSignature();
		}

		ClusterID getClusterID(EntityID id)
		{
			return m_entites[id]->getID();
		}

		void setCluster(EntityID id, ObserverPtr<Cluster> cluster)
		{
			m_entites[id] = cluster;
		}
	};
}

#endif