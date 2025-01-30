module;
#include <vector>
#include <variant>
#include <Utility.hpp>
export module Tako.HandleVec;

import Tako.NumberTypes;

namespace tako
{

	template<typename H>
	concept Handle = requires(H h)
	{
		{ h.value } -> std::same_as<U64&>;
	};

	template<class T>
	struct Entry
	{
		std::variant<T, U32> value;
		U32 gen = 0;
	};

	struct GenHandle
	{
		U32 index;
		U32 gen;
	};

	export template<Handle Handle, class T>
	class HandleVec
	{
	public:

		Handle Insert(T&& item)
		{
			if (m_freeHandle)
			{
				auto index = m_freeHandle;
				auto& entry = m_data[m_freeHandle - 1];
				ASSERT(std::holds_alternative<U32>(entry.value));
				m_freeHandle = *std::get_if<U32>(&entry.value);
				entry.value = std::forward<T>(item);

				return MakeHandle(index, entry.gen);
			}

			constexpr U32 gen = 0;
			m_data.emplace_back(std::forward<T>(item), gen);
			return MakeHandle(m_data.size(), gen);
		}

		void Remove(Handle handle)
		{
			GenHandle h = std::bit_cast<GenHandle>(handle.value);
			auto entry = GetEntry(h);
			ASSERT(entry.gen == h.gen);
			entry.value = m_freeHandle;
			m_freeHandle = h.index;
			++entry.gen;
		}

		T& operator[](const Handle& handle)
		{
			GenHandle h = std::bit_cast<GenHandle>(handle.value);
			auto entry = GetEntry(h);
			ASSERT(entry.gen == h.gen);
			ASSERT(std::holds_alternative<T>(entry.value));
			return *std::get_if<T>(&entry.value);
		}
	private:
		std::vector<Entry<T>> m_data;
		U32 m_freeHandle = 0;

		Handle MakeHandle(const U32 index, const U32 gen)
		{
			GenHandle h;
			h.index = index;
			h.gen = gen;
			Handle handle;
			handle.value = std::bit_cast<U64>(h);
			return handle;
		}

		Entry<T>& GetEntry(const GenHandle& handle)
		{
			ASSERT(handle.index - 1 < m_data.size());
			return m_data[handle.index - 1];
		}
	};
}
