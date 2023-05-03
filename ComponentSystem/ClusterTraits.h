#ifndef CLUSTERTRAITS_H
#define	CLUSTERTRAITS_H

#include "Core.h"

#include "Cluster.h"
#include "EntityContainer.h"

namespace CECS
{
	struct ClusterTraits
	{
		static void carryEntity(EntityID newEntity, EntityID oldEntity, Cluster& current, Cluster& destination)
		{
			ArrayContainer& currentArrays{ current.m_arrays };
			Index oldIndex{ current.m_indices[oldEntity] };
			destination.addEntity(newEntity);
			for (auto& pair : destination.m_arrays)
			{
				auto found{ currentArrays.find(pair.first) };
				if (found != currentArrays.end())
				{
					pair.second->carryFrom(*found->second, oldIndex);
				}
			}
			current.removeEntity(oldEntity);
		}

		static void copyEntity(EntityID newEntity, EntityID oldEntity, Cluster& current, Cluster& destination)
		{
			ArrayContainer& currentArrays{ current.m_arrays };
			Index oldIndex{ current.m_indices[oldEntity] };
			destination.addEntity(newEntity);
			for (auto& pair : destination.m_arrays)
			{
				auto found{ currentArrays.find(pair.first) };
				if (found != currentArrays.end())
				{
					pair.second->copyFrom(*found->second, oldIndex);
				}
			}
		}

	};

}

#endif
