#ifndef OBSERVERPTR_H
#define OBSERVERPTR_H

namespace CECS
{

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

}

#endif