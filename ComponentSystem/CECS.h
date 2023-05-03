#ifndef CECS_H
#define CECS_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <bitset>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <tuple>
#include <unordered_map>
#include <set>

namespace CECS
{
	using EntityID = uint32_t;
	using ComponentID = uint32_t;
	using ClusterID = uint16_t;
	using Index = uint32_t;
	using TypeName = std::string;

	inline constexpr EntityID nullent{ 0 };
	inline constexpr ClusterID nullcluster{ 0 };

	inline constexpr size_t BITSET_SIZE{ 64 };


	template<typename T>
	class ObserverPtr
	{
	private:
		T* m_ptr;

	public:
		ObserverPtr() noexcept
			:m_ptr{ nullptr }
		{
		}

		ObserverPtr(T* ptr) noexcept
			:m_ptr{ ptr }
		{
		}

		ObserverPtr(T& item)
			:m_ptr{ &item }
		{
		}

		ObserverPtr(const ObserverPtr& ptr)
			:m_ptr{ ptr.m_ptr }
		{
		}

		ObserverPtr(ObserverPtr&& ptr) noexcept
			:m_ptr{ ptr.m_ptr }
		{
		}

		ObserverPtr& operator=(const ObserverPtr& ptr)
		{
			m_ptr = ptr.m_ptr;
			return *this;
		}

		ObserverPtr& operator=(ObserverPtr&& ptr) noexcept
		{
			m_ptr = ptr.m_ptr;
			return *this;
		}

		ObserverPtr& operator=(T* ptr)
		{
			m_ptr = ptr;
			return *this;
		}

		T& operator*()
		{
			return *m_ptr;
		}

		T* get()
		{
			return m_ptr;
		}

		T* operator->()
		{
			return m_ptr;
		}

		const T& operator*() const
		{
			return *m_ptr;
		}

		bool isValid() const
		{
			return static_cast<bool>(m_ptr);
		}

		void reset()
		{
			m_ptr = nullptr;
		}

		bool operator==(const ObserverPtr<T>& observePtr) const
		{
			return m_ptr == observePtr;
		}

		bool operator!=(const ObserverPtr<T>& observePtr) const
		{
			return m_ptr != observePtr;
		}
	};


	class Signature
	{
	public:
		using Bitset = std::bitset<BITSET_SIZE>;

	private:
		using DynamicBitset = std::map<Index, Bitset>;

		DynamicBitset m_bits;

	public:
		Signature() = default;

		bool check(ComponentID id) const
		{
			return m_bits.at(id / BITSET_SIZE)[id % BITSET_SIZE];
		}

		void set(ComponentID id, bool value = true)
		{
			auto check{ m_bits.find(id / BITSET_SIZE) };

			if (check == m_bits.end())
			{
				m_bits[id / BITSET_SIZE] = Bitset{};
			}

			Bitset& bitsetBlock{ m_bits[id / BITSET_SIZE] };
			bitsetBlock.set(id % BITSET_SIZE, value);

			if (!bitsetBlock.any())
			{
				m_bits.erase(id / BITSET_SIZE);
			}
		}

		bool any() const
		{
			return !m_bits.empty();
		}

		Bitset& bitset(Index index)
		{
			return m_bits[index];
		}

		const Bitset& bitset(Index index) const
		{
			return m_bits.at(index);
		}

		bool subset(const Signature& aim) const
		{
			for (auto& pair : m_bits)
			{
				auto check{ aim.m_bits.find(pair.first) };
				if (check == aim.m_bits.end() ||
					(bitset(pair.first) & aim.bitset(pair.first)) != bitset(pair.first))
				{
					return false;
				}
			}
			return true;
		}

		bool anyMatch(const Signature& aim) const
		{
			for (auto& pair : aim.m_bits)
			{
				auto check{ m_bits.find(pair.first) };
				if (check != m_bits.end() &&
					(bitset(pair.first) & aim.bitset(pair.first)).any())
				{
					return true;
				}
			}
			return false;
		}

		bool operator==(const Signature& right) const
		{
			for (auto& pair : right.m_bits)
			{
				auto check{ m_bits.find(pair.first) };
				if (check == m_bits.end() ||
					(bitset(pair.first) == right.bitset(pair.first)))
				{
					return true;
				}
			}
			return false;
		}

		bool operator!=(const Signature& right) const
		{
			return !(*this == right);
		}

		void operator+=(const Signature& signature)
		{
			for (const auto& pair : signature.m_bits)
			{
				auto result{ m_bits.find(pair.first) };
				if (result != m_bits.end())
				{
					result->second = result->second | pair.second;
				}
				else
				{
					m_bits[pair.first] = pair.second;
				}
			}
		}

		std::vector<ComponentID> getComponents() const
		{
			std::vector<ComponentID> out;
			for (auto& pair : m_bits)
			{
				for (ComponentID index{}; index < BITSET_SIZE; ++index)
				{
					if (pair.second.test(index))
					{
						out.push_back(pair.first * BITSET_SIZE + index);
					}
				}
			}

			return out;
		}

		const DynamicBitset& getBitsets() const
		{
			return m_bits;
		}

	};


	inline constexpr size_t ARRAY_BYTES{ 500000 };

	template<typename T>
	class RigitAllocator : public std::allocator<T>
	{
	public:
		size_t chunkSize()
		{
			size_t size{ ARRAY_BYTES / sizeof(T) };
			if (size == 0) {
				return 1;
			}
			return size;
		}
	};


	template<typename T>
	class RigitIterator
	{
	private:
		using MultiArray = T**;

		MultiArray m_arrays;
		size_t m_arraySize;
		size_t m_itemIndex;

	public:
		RigitIterator(MultiArray arrays, size_t arraySize, size_t itemIndex)
			:m_arrays{ arrays }, m_arraySize{ arraySize }, m_itemIndex{ itemIndex }
		{
		}

		T& operator*()
		{
			return m_arrays[m_itemIndex / m_arraySize][m_itemIndex % m_arraySize];
		}

		const T& operator*() const
		{
			return m_arrays[m_itemIndex / m_arraySize][m_itemIndex % m_arraySize];
		}

		RigitIterator& operator++()
		{
			++m_itemIndex;
			return *this;
		}

		bool operator==(const RigitIterator& right) const
		{
			return m_itemIndex == right.m_itemIndex;
		}

		bool operator!=(const RigitIterator& right) const
		{
			return m_itemIndex != right.m_itemIndex;
		}
	};

	template<typename T, typename Allocator = RigitAllocator<T>>
	class RigitArray
	{
	private:
		using Array = T*;
		using ArrayContainer = std::vector<Array>;

		ArrayContainer m_arrays;
		Allocator m_allocator;
		size_t m_arraySize;
		size_t m_itemCount{};

	public:
		RigitArray() 
			:m_arraySize{ m_allocator.chunkSize() }
		{
		}

		RigitArray(const RigitArray& rigitArray) : RigitArray(std::move(rigitArray.copy()))
		{
		}

		RigitArray(RigitArray&& rigitArray) noexcept :
			m_arrays{ std::move(rigitArray.m_arrays) },
			m_allocator{ std::move(rigitArray.m_allocator) },
			m_arraySize{ rigitArray.m_arraySize },
			m_itemCount{ rigitArray.m_itemCount }
		{
			rigitArray.m_itemCount = 0;
		}

		RigitArray& operator=(const RigitArray& rigitArray)
		{
			clear();
			*this = std::move(rigitArray.copy());
			return *this;
		}

		RigitArray& operator=(RigitArray&& rigitArray) noexcept
		{
			clear();
			m_arrays = std::move(rigitArray.m_arrays);
			m_allocator = std::move(rigitArray.m_allocator);
			m_arraySize = rigitArray.m_arraySize;
			m_itemCount = rigitArray.m_itemCount;
			rigitArray.m_itemCount = 0;
			return *this;
		}

		virtual ~RigitArray()
		{
			clear();
		}

		void pushBack(const T& item)
		{
			pushBack(T{ item });
		}

		void pushBack(T&& item)
		{
			increaseCapacity(m_itemCount + 1);
			constuct(m_itemCount, std::move(item));
			++m_itemCount;
		}

		void popBack()
		{
			destroy(m_itemCount - 1);
			--m_itemCount;
			decreaseCapacity(m_itemCount);
		}

		T& get(size_t index)
		{
			return m_arrays[index / m_arraySize][index % m_arraySize];
		}

		const T& get(size_t index) const
		{
			return m_arrays[index / m_arraySize][index % m_arraySize];
		}

		void set(size_t index, const T& item)
		{
			get(index) = item;
		}

		void set(size_t index, T&& item)
		{
			get(index) = std::move(item);
		}

		T& operator[](size_t index)
		{
			return get(index);
		}

		const T& operator[](size_t index) const
		{
			return get(index);
		}

		T& back()
		{
			return get(m_itemCount - 1);
		}

		const T& back() const
		{
			return get(m_itemCount - 1);
		}

		void swap(size_t left, size_t right)
		{
			std::swap(get(left), get(right));
		}

		void clear()
		{
			for (size_t index{}; index < m_itemCount; ++index)
			{
				destroy(index);
			}
			decreaseCapacity(0);
			m_itemCount = 0;
		}

		size_t size() const
		{
			return m_itemCount;
		}

		size_t capacity() const
		{
			return m_arraySize * m_arrays.size();
		}

		bool empty() const
		{
			return !static_cast<bool>(m_itemCount);
		}

		RigitIterator<T> begin()
		{
			return RigitIterator<T>{m_arrays.data(), m_arraySize, 0};
		}

		RigitIterator<T> end()
		{
			return RigitIterator<T>{nullptr, 0, m_itemCount};
		}

		RigitArray copy() const
		{
			RigitArray<T, Allocator> out;
			for (size_t index{}; index < m_itemCount; ++index)
			{
				out.pushBack(get(index));
			}
			return out;
		}

	private:
		void constuct(size_t index, T&& item)
		{
			std::allocator_traits<Allocator>::construct(m_allocator, &get(index), std::move(item));
		}

		void destroy(size_t index)
		{
			std::allocator_traits<Allocator>::destroy(m_allocator, &get(index));
		}

		void increaseCapacity(size_t newCapacity)
		{
			while (newCapacity > capacity())
			{
				m_arrays.push_back(std::allocator_traits<Allocator>::allocate(m_allocator, m_arraySize));
			}
		}

		void decreaseCapacity(size_t newCapacity)
		{
			if (newCapacity)
			{
				newCapacity += m_arraySize;
			}

			while (capacity() > newCapacity)
			{
				Array removal{ m_arrays.back() };
				std::allocator_traits<Allocator>::deallocate(m_allocator, removal, m_arraySize);
				m_arrays.pop_back();
			}
		}
	};


	template<typename T>
	class Indexer
	{
	private:
		using EmptyIndexContainer = RigitArray<T>;

		T m_nextIndex;
		EmptyIndexContainer m_emptyIndices;

	public:
		Indexer(T start = 0)
			:m_nextIndex{ start }
		{
		}

		T createIndex()
		{
			if (m_emptyIndices.empty())
			{
				return m_nextIndex++;
			}

			T out{ m_emptyIndices.back() };
			m_emptyIndices.popBack();

			return out;
		}

		void releaseIndex(T index)
		{
			m_emptyIndices.pushBack(index);
		}

	};


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


	template<typename T>
	struct Hash
	{
		constexpr size_t operator()(const T& arg) const
		{
			size_t hash{ static_cast<size_t>(reinterpret_cast<std::uintptr_t>(&arg)) };
			hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
			hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
			hash = ((hash >> 16) ^ hash);
			return hash;
		}
	};

	template<>
	struct Hash<uint32_t>
	{
		constexpr uint32_t operator()(uint32_t arg) const
		{
			return arg;
		}
	};

	template<>
	struct Hash<uint16_t>
	{
		constexpr uint16_t operator()(uint16_t arg) const
		{
			return arg;
		}
	};

	template<>
	struct Hash<size_t>
	{
		constexpr size_t operator()(size_t arg) const
		{
			return arg;
		}
	};

	template<>
	struct Hash<std::string>
	{
		constexpr uint32_t operator()(const std::string& s) const
		{
			return operator()(s.c_str());
		}

		constexpr uint32_t operator()(const char* s) const
		{
			uint32_t hash{};

			for (uint32_t index{}; s[index]; ++index)
			{
				hash += (index + 1) * s[index];
			}

			return hash;
		}
	};

	template<>
	struct Hash<Signature>
	{
		size_t operator()(const Signature& signature) const
		{
			size_t out{};
			for (const auto& pair : signature.getBitsets())
			{
				out += (pair.first + 1) * pair.second.to_ullong();
			}
			return out;
		}
	};


	using ArrayContainer = std::unordered_map<TypeName, ComponentArrayBase, Hash<std::string>>;

	class Cluster
	{
	private:
		using EntityMap = std::unordered_map<EntityID, Index, Hash<EntityID>>;
		using ReverseMap = RigitArray<EntityID>;

		ArrayContainer m_arrays;
		EntityMap m_indices;
		ReverseMap m_reverse;
		ClusterID m_id{ nullcluster };
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
				:m_ids{ &cluster.m_reverse }
			{
				for (auto& name : names)
				{
					m_arrays.push_back(cluster.m_arrays[name].get());
				}
			}

			std::tuple<EntityID, Args&...> getItems(Index index)
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
			:m_id{ id }, m_signature{ signature }
		{
		}

		Cluster(const Cluster& cluster)
			:m_indices{ cluster.m_indices }, m_reverse{ cluster.m_reverse }, m_signature{ cluster.m_signature }
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


	class IRecipie
	{
	public:
		virtual ComponentArrayBase createArray() const = 0;
	};

	template<typename T>
	class Recipie : public IRecipie
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
		Indexer<ClusterID> m_indexer{ 1 };

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
				return createCluster(signature, components);
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

		void addEntity(EntityID id, ObserverPtr<Cluster> cluster = nullptr)
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


	class Pool
	{
	private:
		ComponentRegister m_register;
		ClusterContainer m_clusters;
		EntityContainer m_entites;
		Indexer<EntityID> m_indexer{ 1 };

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
			newCluster->addComponent(std::move(component), id, name);
		}

		void removeComponent(EntityID id, const TypeName& name)
		{
			ObserverPtr<Cluster>& oldCluster{ m_entites.getEntityCluster(id) };
			Signature signature{ oldCluster->getSignature() };
			signature.set(m_register.getID(name), false);

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
			((m_register.registerComponent<Args>(names[index]), out.set(m_register.getID(names[index])), ++index), ...);
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
			addComponent<T>(id, T{ component }, typeid(T).name());
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
			return instance().m_pool->hasComponent(id, name);
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
			for (const TypeName& name : typeNames)
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
				:m_cluster{ cluster }, m_typeNames{ typeNames }, m_index{ index }
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
			return ClusterIterator<Args...>{ m_cluster, m_typeNames, 0 };
		}

		ClusterIterator<Args...> end()
		{
			return ClusterIterator<Args...>{ m_cluster, m_typeNames, static_cast<Index>(m_cluster->size()) };
		}
	};


	template<typename... Args>
	class PoolView
	{
	private:
		template<typename... Args>
		class PoolIterator
		{
		private:
			ClusterContainer::ClusterWindow& m_clusters;
			std::vector<TypeName>& m_typeNames;
			Index m_clusterIndex;
			Index m_itemIndex;
			Cluster::Cache<Args...> m_cache;

		public:
			PoolIterator(
				ClusterContainer::ClusterWindow& clusters,
				std::vector<TypeName>& typeNames,
				Index clusterIndex,
				Index itemIndex)
				:m_clusters{ clusters }, m_typeNames{ typeNames }, m_clusterIndex{ clusterIndex }, m_itemIndex{ itemIndex }
			{
				if (m_clusterIndex < m_clusters.size())
				{
					m_cache = Cluster::Cache<Args...>(*m_clusters[m_clusterIndex], m_typeNames);
				}
			}

			PoolIterator& operator++()
			{
				++m_itemIndex;
				if (m_itemIndex == m_clusters[m_clusterIndex]->size())
				{
					m_itemIndex = 0;
					m_clusterIndex++;
					if (m_clusterIndex < m_clusters.size())
					{
						m_cache = Cluster::Cache<Args...>(*m_clusters[m_clusterIndex], m_typeNames);
					}
				}
				return *this;
			}

			bool operator==(const PoolIterator& right)
			{
				return right.m_clusterIndex == m_clusterIndex && right.m_itemIndex == m_itemIndex;
			}

			bool operator!=(const PoolIterator& right)
			{
				return !(*this == right);
			}

			std::tuple<EntityID, Args&...> operator*()
			{
				return m_cache.getItems(m_itemIndex);
			}
		};

		ClusterContainer::ClusterWindow m_clusters;
		std::vector<TypeName> m_typeNames;

	public:
		PoolView()
		{
			(m_typeNames.push_back(typeid(Args).name()), ...);
			setup(Signature{});
		}

		PoolView(const std::vector<TypeName>& needed)
		{
			m_typeNames = needed;
			setup(Signature{});
		}

		PoolView(const std::vector<TypeName>& needed, const std::vector<TypeName>& noNeeded)
		{
			m_typeNames = needed;
			setup(Accessor::createSignature(noNeeded));
		}

		PoolIterator<Args...> begin()
		{
			return PoolIterator<Args...>(m_clusters, m_typeNames, 0, 0);
		}

		PoolIterator<Args...> end()
		{
			return PoolIterator<Args...>(m_clusters, m_typeNames, static_cast<Index>(m_clusters.size()), 0);
		}

	private:
		void setup(const Signature& noIntersection)
		{
			ClusterContainer& container{ Accessor::getClusterContainer() };
			m_clusters = container.getClusters(Accessor::createSignature(m_typeNames), noIntersection);
		}
	};


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
			Accessor::addComponents<Args...>(m_id, std::move(components), names);
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


	class ISystem
	{
	public:
		ISystem() = default;

		virtual ~ISystem() = default;

		virtual void initilize()
		{
		}

		virtual void terminate()
		{
		}

		virtual void update() = 0;
	};
}

#endif