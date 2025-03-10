module;
#include "Utility.hpp"
#include <string>
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
		constexpr CStringView(const std::string& str) : m_str(str.c_str()), m_len(str.length()) {}

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
		constexpr StringView(const std::string& str) : m_str(str.c_str()), m_len(str.length()), m_isTerminated(true) {}
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
			return ToStringView();
		}

		constexpr CStringView ToCStringView() const noexcept
		{
			ASSERT(m_isTerminated);
			return CStringView(m_str, m_len);
		}

		constexpr std::string_view ToStringView() const noexcept
		{
			return {m_str, m_len};
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
		constexpr CStringBuffer() : m_view(nullptr) {}
		constexpr CStringBuffer(std::string_view view)
		{
			m_str = view;
			m_view = m_str;
		}

		constexpr CStringBuffer(const StringView& view)
		{
			if (view.IsCString())
			{
				m_view = view.ToCStringView();
			}
			else
			{
				m_str = view;
				m_view = m_str;
			}
		}

		constexpr const char* c_str() const noexcept
		{
			return m_view.c_str();
		}

		constexpr operator CStringView() const noexcept
		{
			return m_view;
		}
	protected:
		CStringView m_view;
		std::string m_str;
	};
}
