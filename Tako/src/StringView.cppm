module;
#include "Utility.hpp"
#include <string_view>
#include <optional>
export module Tako.StringView;

namespace tako
{
	export class CStringView
	{
	public:
		constexpr CStringView() : m_str(nullptr), m_len(0) {}
		constexpr CStringView(const char* str) : m_str(str), m_len(std::char_traits<char>::length(str)) {}
		constexpr CStringView(const char* str, size_t len) : m_str(str), m_len(len) {}

		constexpr const char* data() const noexcept
		{
			return m_str;
		}

		constexpr std::size_t size() const noexcept
		{
			return m_len;
		}

		constexpr operator std::string_view() const noexcept
		{
			return {m_str, m_len};
		}

		constexpr operator const char*() const noexcept
		{
			return m_str;
		}

		constexpr const char* c_str() const noexcept
		{
			return m_str;
		}

	protected:
		std::size_t m_len;
		const char* m_str;
	};

	export class StringView
	{
	public:
		constexpr StringView() : m_str(nullptr), m_len(0), m_isTerminated(true) {}
		constexpr StringView(const char* str) : m_str(str), m_len(std::char_traits<char>::length(str)), m_isTerminated(true) {}
		constexpr StringView(std::string_view str) : m_str(str.data()), m_len(str.size()), m_isTerminated(false) {}
		constexpr StringView(CStringView str) : m_str(str), m_len(str.size()), m_isTerminated(true) {}

		constexpr const char* data() const noexcept
		{
			return m_str;
		}

		constexpr std::size_t size() const noexcept
		{
			return m_len;
		}

		constexpr operator std::string_view() const noexcept
		{
			return {m_str, m_len};
		}

		CStringView ToCStringView()
		{
			ASSERT(m_isTerminated);
			return CStringView(m_str, m_len);
		}

		constexpr bool IsCString() const noexcept
		{
			return m_isTerminated;
		}
	protected:
		std::size_t m_len;
		const char* m_str;
		bool m_isTerminated;
	};

	export class CStringBuffer
	{
	public:
		constexpr CStringBuffer() : m_cstr(nullptr) {}
		constexpr CStringBuffer(std::string_view view)
		{
			m_str = view;
			m_cstr = m_str.c_str();
		}

		constexpr CStringBuffer(const StringView& view)
		{
			if (view.IsCString())
			{
				m_cstr = view.data();
			}
			else
			{
				m_str = view;
				m_cstr = m_str.c_str();
			}
		}

		constexpr const char* c_str() const noexcept
		{
			return m_cstr;
		}
	protected:
		const char* m_cstr;
		std::string m_str;
	};
}
