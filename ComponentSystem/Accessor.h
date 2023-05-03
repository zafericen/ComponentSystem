#ifndef	ACCESSOR_H
#define ACCESSOR_H

#include <unordered_map>
#include <string>
#include <memory>
#include <type_traits>

#include "Core.h"

#include "Pool.h"

namespace CECS
{
	class Accessor
	{
	private:
		using PoolContainer = std::unordered_map<std::string, std::unique_ptr<Pool>>;
		
		PoolContainer m_pools;
		ObserverPtr<Pool> m_pool;

		Accessor()
		{
			m_pools["default"] = std::make_unique<Pool>();
			m_pool = m_pools["default"].get();
		}

		static Accessor& instance()
		{
			static Accessor manager;

			return manager;
		}

		template<typename... Args>
		friend class ClusterView;

		template<typename... Args>
		friend class PoolView;

	public:
		Accessor(const Accessor& manager) = delete;

		static EntityID createEntity()
		{
			return instance().m_pool->createEntity();
		}

		static void destroyEntity(EntityID id)
		{
			instance().m_pool->destroyEntity(id);
		}

		template <typename T>
		static void addComponent(EntityID id, const T& component)
		{
			addComponent<T>(id, T{component}, typeid(T).name());
		}

		template <typename T>
		static void addComponent(EntityID id, T&& component)
		{
			addComponent<T>(id, std::move(component), typeid(T).name());
		}

		template <typename T>
		static void addComponent(EntityID id, const T& component, const TypeName& typeName)
		{
			addComponent<T>(id, T{ component }, typeName);
		}

		template <typename T>
		static void addComponent(EntityID id, T&& component, const TypeName& typeName)
		{
			instance().m_pool->addComponent<T>(id, std::move(component), typeName);
		}

		template<typename T>
		static void removeComponent(EntityID id)
		{
			removeComponent(id, typeid(T).name());
		}

		static void removeComponent(EntityID id, const TypeName& name)
		{
			instance().m_pool->removeComponent(id, name);
		}

		template<typename T>
		static T& getComponent(EntityID id)
		{
			return getComponent<T>(id, typeid(T).name());
		}

		template<typename T>
		static T& getComponent(EntityID id, const TypeName& name)
		{
			return instance().m_pool->getComponent<T>(id, name);
		}

		template <typename T>
		static void setComponent(EntityID id, const T& component)
		{
			setComponent<T>(id, T{ component }, typeid(T).name());
		}

		template <typename T>
		static void setComponent(EntityID id, T&& component)
		{
			setComponent<T>(id, std::move(component), typeid(T).name());
		}

		template <typename T>
		static void setComponent(EntityID id, const T& component, const TypeName& typeName)
		{
			setComponent<T>(id, T{ component }, typeName);
		}

		template <typename T>
		static void setComponent(EntityID id, T&& component, const TypeName& typeName)
		{
			instance().m_pool->setComponent<T>(id, std::move(component), typeName);
		}

		template<typename... Args>
		static void addComponents(EntityID id, Args&&... components)
		{
			std::vector<TypeName> typeNames;
			(typeNames.push_back(typeid(Args).name()), ...);
			instance().m_pool->addComponents<Args...>(id, std::move(components)..., typeNames);
		}

		template<typename... Args>
		static void addComponents(EntityID id, Args&&... components, const std::vector<TypeName>& names)
		{
			instance().m_pool->addComponents<Args...>(id, std::move(components)..., names);
		}

		static EntityID copyEntity(EntityID old)
		{
			EntityID out{ createEntity() };
			instance().m_pool->copyEntity(out, old);
			return out;
		}

		template<typename T>
		static bool hasComponent(EntityID id)
		{
			return hasComponent(id, typeid(T).name());
		}

		static bool hasComponent(EntityID id, const TypeName& name)
		{
			return instance().m_pool->hasComponent(id,name);
		}

		template<typename... Args>
		static Signature createSignature()
		{
			std::vector<TypeName> typeNames;
			(typeNames.push_back(typeid(Args).name()), ...);
			createSignature<Args...>(typeNames);
		}

		template<typename... Args>
		static Signature createSignature(const std::vector<TypeName>& typeNames)
		{
			return instance().m_pool->createSignature<Args...>(typeNames);
		}

		static Signature createSignature(const std::vector<TypeName>& typeNames)
		{
			Signature out;
			ComponentRegister& components{ getComponentRegister() };
			for (const TypeName& name: typeNames)
			{
				out.set(components.getID(name));
			}
			return out;
		}

		static ObserverPtr<Pool> getPool()
		{
			return instance().m_pool;
		}

		static ClusterID getClusterID(EntityID id)
		{
			return instance().m_pool->m_entites.getClusterID(id);
		}

		static const Signature& getSignature(EntityID id)
		{
			return instance().m_pool->m_entites.getSignature(id);
		}

		static void carryEntity(EntityID id, ClusterID newCluster)
		{
			instance().m_pool->carryEntity(id, newCluster);
		}

	private:
		static ClusterContainer& getClusterContainer()
		{
			return instance().m_pool->m_clusters;
		}

		static ComponentRegister& getComponentRegister()
		{
			return instance().m_pool->m_register;
		}
	};
}

#endif