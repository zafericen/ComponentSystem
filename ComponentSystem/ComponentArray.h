#ifndef COMPONENTARRAY_H
#define COMPONENTARRAY_H

#include <memory>
#include <type_traits>

#include "Core.h"

#include "RigitArray.h"

namespace CECS
{
	class IComponentArray;
	using ComponentArrayBase = std::unique_ptr<IComponentArray>;

	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;

		virtual void removeComponent() = 0;

		virtual void swapComponents(Index left, Index right) = 0;

		virtual void carryFrom(IComponentArray& origin, Index index) = 0;

		virtual void copyFrom(const IComponentArray& origin, Index index) = 0;

		virtual ComponentArrayBase copy() const = 0;

	};

	template<typename Component>
	class ComponentArray : public IComponentArray
	{
	private:
		RigitArray<Component> components;

	public:
		ComponentArray() = default;

		ComponentArray(const RigitArray<Component>& rigitArray) : components{ rigitArray }
		{
		}

		ComponentArray(const ComponentArray& componentArray) = default;

		ComponentArray(ComponentArray&& componentArray) = default;

		ComponentArray& operator=(const ComponentArray& componentArray) = default;

		ComponentArray& operator=(ComponentArray&& componentArray) = default;

		~ComponentArray() = default;

		void addComponent(const Component& component)
		{
			components.pushBack(Component{ component });
		}

		void addComponent(Component&& component)
		{
			components.pushBack(std::move(component));
		}

		void removeComponent() override
		{
			components.popBack();
		}

		Component& getComponent(Index index)
		{
			return components.get(index);
		}

		const Component& getComponent(Index index) const
		{
			return components.get(index);
		}

		void setComponent(Index index, Component&& component)
		{
			components.set(index, std::move(component));
		}

		void swapComponents(Index left, Index right) override
		{
			components.swap(left, right);
		}

		void carryFrom(IComponentArray& origin, Index index) override
		{
			addComponent(std::move(static_cast<ComponentArray<Component>&>(origin).getComponent(index)));
		}

		void copyFrom(const IComponentArray& origin, Index index) override
		{
			addComponent(Component{ static_cast<const ComponentArray<Component>&>(origin).getComponent(index) });
		}

		ComponentArrayBase copy() const override
		{
			return std::make_unique<ComponentArray<Component>>(ComponentArray<Component>{components});
		}

		size_t size() const
		{
			return components.size();
		}

		bool empty() const
		{
			return static_cast<bool>(components.size());
		}

		RigitIterator<Component> begin()
		{
			return components.begin();
		}

		RigitIterator<Component> end()
		{
			return components.end();
		}

	};
}

#endif