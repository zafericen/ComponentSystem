#ifndef COMPONENTREGISTER_H
#define	COMPONENTREGISTER_H

#include <unordered_map>
#include <memory>
#include <vector>

#include "Core.h"

#include "Indexer.h"
#include "RigitArray.h"
#include "ObserverPtr.h"
#include "ComponentArray.h"

namespace CECS
{
	class IRecipie
	{
	public:
		virtual ComponentArrayBase createArray() const = 0;
	};

	template<typename T>
	class Recipie: public IRecipie
	{
	public:
		ComponentArrayBase createArray() const override
		{
			return std::make_unique<ComponentArray<T>>();
		}
	};

	using RecipieBase = std::unique_ptr<IRecipie>;

	class ComponentRegister
	{
	private:
		using RecipieContainer = std::unordered_map<ComponentID, RecipieBase>;
		using IDConverter = std::unordered_map<TypeName, ComponentID>;

		Indexer<ComponentID> m_indexer{};
		RecipieContainer m_recipies;
		IDConverter m_componentIDS;
		std::vector<TypeName> m_typeNames;

	public:
		ComponentRegister() = default;

		template<typename T>
		void registerComponent(const TypeName& name)
		{
			if (!registered(name))
			{
				Index id{ m_indexer.createIndex() };
				m_componentIDS[name] = id;
				m_typeNames.push_back(name);
				m_recipies[id] = std::make_unique<Recipie<T>>();
			}
		}

		bool registered(const TypeName& name) const
		{
			return m_componentIDS.contains(name);
		}

		ComponentID getID(const TypeName& name) const
		{
			return m_componentIDS.at(name);
		}

		TypeName getName(ComponentID id) const
		{
			return m_typeNames[id];
		}

		ObserverPtr<IRecipie> getRecipie(ComponentID id) const
		{
			return m_recipies.at(id).get();
		}
	};

}

#endif