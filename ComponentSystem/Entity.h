#ifndef ENTITY_H 
#define	ENTITY_H

#include "Core.h";

#include "Accessor.h"

namespace CECS
{
	class Entity
	{
	private:
		EntityID m_id;

	public:
		Entity()
		{
			m_id = Accessor::createEntity();
		}

		Entity(const Entity& entity)
		{
			m_id = Accessor::copyEntity(entity.m_id);
		}

		Entity(Entity&& entity) noexcept
		{
			m_id = entity.m_id;
			entity.m_id = nullent;
		}

		Entity& operator=(const Entity& entity)
		{
			Accessor::destroyEntity(m_id);
			m_id = Accessor::copyEntity(m_id);
			return *this;
		}

		Entity& operator=(Entity&& entity) noexcept
		{
			Accessor::destroyEntity(m_id);
			m_id = entity.m_id;
			entity.m_id = nullent;
			return *this;
		}

		virtual ~Entity()
		{
			Accessor::destroyEntity(m_id);
		}

		template <typename T>
		void addComponent(const T& component)
		{
			Accessor::addComponent<T>(m_id, T{ component }, typeid(T).name());
		}

		template <typename T>
		void addComponent(T&& component)
		{
			Accessor::addComponent<T>(m_id, std::move(component), typeid(T).name());
		}

		template <typename T>
		void addComponent(const T& component, const TypeName& typeName)
		{
			Accessor::addComponent<T>(m_id, T{ component }, typeName);
		}

		template <typename T>
		void addComponent(T&& component, const TypeName& typeName)
		{
			Accessor::addComponent<T>(m_id, std::move(component), typeName);
		}

		template<typename T>
		void removeComponent()
		{
			Accessor::removeComponent(m_id, typeid(T).name());
		}

		void removeComponent(const TypeName& name)
		{
			Accessor::removeComponent(m_id, name);
		}

		template<typename T>
		T& getComponent()
		{
			return Accessor::getComponent<T>(m_id, typeid(T).name());
		}

		template<typename T>
		T& getComponent(const TypeName& name)
		{
			return Accessor::getComponent<T>(m_id, name);
		}

		template <typename T>
		void setComponent(const T& component)
		{
			Accessor::setComponent<T>(m_id, T{ component }, typeid(T).name());
		}

		template <typename T>
		void setComponent(T&& component)
		{
			Accessor::setComponent<T>(m_id, std::move(component), typeid(T).name());
		}

		template <typename T>
		void setComponent(const T& component, const TypeName& typeName)
		{
			Accessor::setComponent<T>(m_id, T{ component }, typeName);
		}

		template <typename T>
		void setComponent(T&& component, const TypeName& typeName)
		{
			Accessor::setComponent<T>(m_id, std::move(component), typeName);
		}

		template<typename... Args>
		void addComponents(Args&&... components)
		{
			Accessor::addComponents<Args...>(m_id, std::move(components));
		}

		template<typename... Args>
		void addComponents(Args&&... components, const std::vector<TypeName>& names)
		{
			Accessor::addComponents<Args...>(m_id, std::move(components),names);
		}

		template<typename T>
		bool hasComponent()
		{
			return Accessor::hasComponent<T>(m_id);
		}

		bool hasComponent(const TypeName& name)
		{
			return Accessor::hasComponent(m_id, name);
		}

		EntityID getID() const
		{
			return m_id;
		}

		ClusterID getClusterID() const
		{
			return Accessor::getClusterID(m_id);
		}

		const Signature& getSignature() const
		{
			return Accessor::getSignature(m_id);
		}
	};
}

#endif