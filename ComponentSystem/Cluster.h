#ifndef CLUSTER_H
#define CLUSTER_H

#include <vector>
#include <tuple>
#include <unordered_map>

#include "Core.h"

#include "Signature.h"
#include "RigitArray.h"
#include "ObserverPtr.h"
#include "ComponentArray.h"
#include "Algorithm.h"

namespace CECS
{

	using ArrayContainer = std::unordered_map<TypeName,ComponentArrayBase,Hash<std::string>>;

	class Cluster
	{
	private:
		using EntityMap = std::unordered_map<EntityID, Index, Hash<EntityID>>;
		using ReverseMap = RigitArray<EntityID>;

		ArrayContainer m_arrays;
		EntityMap m_indices;
		ReverseMap m_reverse;
		ClusterID m_id{nullcluster};
		Signature m_signature;

		friend struct ClusterTraits;

	public:
		template<typename... Args>
		struct Cache
		{
		private:
			std::vector<ObserverPtr<IComponentArray>> m_arrays;
			ObserverPtr<RigitArray<EntityID>> m_ids;

		public:
			Cache() = default;

			Cache(Cluster& cluster, const std::vector<TypeName>& names)
				:m_ids{&cluster.m_reverse}
			{
				for (auto& name : names)
				{
					m_arrays.push_back(cluster.m_arrays[name].get());
				}
			}

			std::tuple<EntityID,Args&...> getItems(Index index)
			{
				size_t arrayIndex{ m_arrays.size() };
				return std::tuple<EntityID, Args&...>(m_ids->get(index), (--arrayIndex, get<Args>(m_arrays[arrayIndex], index))...);
			}

		private:
			template <typename T>
			T& get(ObserverPtr<IComponentArray>& base, Index index)
			{
				return static_cast<ComponentArray<T>*>(base.get())->getComponent(index);
			}
		};

	public:
		Cluster(ClusterID id, const Signature& signature)
			:m_id{ id }, m_signature{signature}
		{
		}

		Cluster(const Cluster& cluster)
			:m_indices{ cluster.m_indices }, m_reverse{ cluster.m_reverse }, m_signature{cluster.m_signature}
		{
			for (auto& arrayPair : cluster.m_arrays)
			{
				m_arrays[arrayPair.first] = arrayPair.second->copy();
			}
		}

		Cluster(Cluster&& cluster) noexcept = default;

		Cluster& operator=(const Cluster& cluster)
		{
			m_indices = cluster.m_indices;
			m_reverse = cluster.m_reverse;
			m_signature = cluster.m_signature;

			m_arrays.clear();
			for (auto& arrayPair : cluster.m_arrays)
			{
				m_arrays[arrayPair.first] = arrayPair.second->copy();
			}
			return *this;
		}

		Cluster& operator=(Cluster&& cluster) noexcept
		{
			m_arrays = std::move(cluster.m_arrays);
			m_indices = std::move(cluster.m_indices);
			m_reverse = std::move(cluster.m_reverse);
			m_signature = std::move(cluster.m_signature);
			m_id = cluster.m_id;
			cluster.m_id = nullcluster;
			return *this;
		}

		~Cluster() = default;
		
		ClusterID getID() const
		{
			return m_id;
		}

		void setID(ClusterID id)
		{
			m_id = id;
		}

		const Signature& getSignature() const
		{
			return m_signature;
		}

		void setSignature(const Signature& signature)
		{
			m_signature = signature;
		}

		void addEntity(EntityID id)
		{
			Index index{ static_cast<Index>(m_indices.size()) };
			m_indices[id] = index;
			m_reverse.pushBack(id);
		}

		template<typename T>
		T& getComponent(EntityID id, const TypeName& typeName)
		{
			return getArray<T>(typeName).getComponent(m_indices[id]);
		}

		template<typename T>
		const T& getComponent(EntityID id, const TypeName& typeName) const
		{
			return getArray<T>(typeName).getComponent(m_indices[id]);
		}

		template<typename... Args>
		std::tuple<EntityID, Args&...> getComponents(EntityID id, const std::vector<TypeName>& typeNames)
		{
			return getDirect<Args...>(m_indices[id], typeNames);
		}

		template<typename T>
		void addComponent(const T& item, EntityID id, const TypeName& typeName)
		{
			addComponent<T>(T{ item }, id, typeName);
		}

		template<typename T>
		void addComponent(T&& item, EntityID id, const TypeName& typeName)
		{
			getArray<T>(typeName).addComponent(std::move(item));
		}

		template<typename... Args>
		void addComponents(Args&&... item, EntityID id, const std::vector<TypeName>& typeNames)
		{
			Index index{ 0 };
			((getArray<Args>(typeNames[index]).addComponent(std::move(item)), ++index), ...);
		}

		void removeEntity(EntityID id)
		{
			Index lastIndex{ static_cast<Index>(m_reverse.size() - 1) };
			for (auto& keyArrayPair : m_arrays)
			{
				Index index{ m_indices[id] };
				keyArrayPair.second->swapComponents(index, lastIndex);
				keyArrayPair.second->removeComponent();
			}

			EntityID swapKey{ m_reverse[lastIndex] };
			m_indices[swapKey] = m_indices[id];
			m_reverse[m_indices[id]] = swapKey;
			m_reverse.popBack();
			m_indices.erase(id);
		}

		template<typename T>
		void setComponent(const T& item, EntityID id, const TypeName& typeName)
		{
			set<T>(T{ item }, id, typeName);
		}

		template<typename T>
		void setComponent(T&& item, EntityID id, const TypeName& typeName)
		{
			getArray<T>(typeName).setComponent(std::move(item), m_indices[id]);
		}

		void addArray(const TypeName& typeName, ComponentArrayBase&& rigitArray)
		{
			m_arrays[typeName] = std::move(rigitArray);
		}

		void removeArray(const TypeName& typeName)
		{
			m_arrays.erase(typeName);
		}

		size_t size() const
		{
			return m_reverse.size();
		}

		bool empty() const
		{
			return m_reverse.empty();
		}

		bool hasEntity(EntityID id) const
		{
			return m_indices.contains(id);
		}

		bool hasArray(const TypeName& typeName) const
		{
			return m_arrays.contains(typeName);
		}

	protected:
		template<typename T>
		ComponentArray<T>& getArray(const TypeName& arrayIndex)
		{
			return *static_cast<ComponentArray<T>*>(m_arrays[arrayIndex].get());
		}
	};

}

#endif