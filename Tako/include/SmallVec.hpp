#pragma once
#include "NumberTypes.hpp"

namespace tako
{
	template<typename T, size_t startCapacity = 10>
	class SmallVec
	{
	public:
		SmallVec()
		{
			m_length = 0;
			m_capacity = startCapacity;
			m_data = reinterpret_cast<T*>(&m_reserved[0]);
		}

		~SmallVec()
		{
			for (size_t i = 0; i < m_length; i++)
			{
				m_data[i].~T();
			}
			if (reinterpret_cast<U8*>(m_data) != &m_reserved[0])
			{
				delete[] reinterpret_cast<U8*>(m_data);
			}
		}

		//TODO: implement copy/move semantics
		SmallVec(SmallVec const& other) = delete;
		SmallVec& operator=(SmallVec const&) = delete;
		SmallVec(SmallVec&& other) noexcept = delete;
		SmallVec& operator=(SmallVec&& other) noexcept = delete;

		size_t GetLength() const
		{
			return m_length;
		}

		size_t GetCapacity() const
		{
			return m_capacity;
		}

		T* GetData() const
		{
			return m_data;
		}

		void Push(const T& element)
		{
			ReserveSizeForPush();
			m_data[m_length] = element;
			m_length++;
		}

		void Push(T&& element)
		{
			ReserveSizeForPush();
			m_data[m_length] = std::move(element);
			m_length++;
		}

		template<typename... Args>
		T& Emplace(Args&& ... args)
		{
			ReserveSizeForPush();
			T& ref = new(m_data[m_length]) T(std::forward<Args>(args)...);
			m_length++;
			return ref;
		}

		void PushArray(const T* arr, size_t num)
		{
			size_t newLen = m_length + num;
			if (newLen > m_capacity)
			{
				size_t newCapacity = m_capacity * 2;
				while (newCapacity < newLen)
				{
					newCapacity *= 2;
				}
				Resize(newCapacity);
			}

			for (int i = 0; i < num; i++)
			{
				m_data[m_length + i] = arr[i];
			}
			m_length = newLen;
		}

		void Pop()
		{
			if (m_length <= 0)
			{
				return;
			}

			m_length--;
			m_data[m_length].~T();
		}

		void Clear()
		{
			for (size_t i = 0; i < m_length; i++)
			{
				m_data[i].~T();
			}
			m_length = 0;
		}

		const T& operator[](size_t index) const
		{
			ASSERT(index < m_length);
			return m_data[index];
		}
	private:
		void ReserveSizeForPush()
		{
			if (m_length >= m_capacity)
			{
				Resize(m_capacity * 2);
			}
		}

		//TODO: Handle downsize
		void Resize(size_t newCapacity)
		{
			T* newData = reinterpret_cast<T*>(new U8[sizeof(T) * newCapacity]);
			if (m_length > 0)
			{
				std::move(&m_data[0], &m_data[m_length], &newData[0]);
			}

			if (reinterpret_cast<U8*>(m_data) != &m_reserved[0])
			{
				delete[] reinterpret_cast<U8*>(m_data);
			}
			m_data = newData;
			m_capacity = newCapacity;
		}

		T* m_data;
		size_t m_length;
		size_t m_capacity;
		U8 m_reserved[sizeof(T) * startCapacity];
	};
}
