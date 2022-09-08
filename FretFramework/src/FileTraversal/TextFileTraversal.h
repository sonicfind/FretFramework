#pragma once
#include "FileTraversal.h"
#include "Song/Modifiers/FloatArray.h"

class TextTraversal : public Traversal
{
	class InvalidLyricExcpetion : std::runtime_error
	{
	public:
		InvalidLyricExcpetion() : std::runtime_error(" no valid lyric string could be extracted") {}
	};

	const unsigned char* m_next;

	size_t m_lineCount = 0;
	std::string_view m_trackName;
	uint32_t m_position = 0;

public:
	TextTraversal(const FilePointers& file);
	bool next();
	void skipTrack() override;

	void setTrackName();
	bool isTrackName(const char* str) const;
	bool cmpTrackName(const std::string_view str);
	std::string getLowercaseTrackName() const;

	void skipWhiteSpace();

	template <typename T>
	bool extract(T& value) noexcept
	{
		static_assert(std::is_integral_v<T>);

		if constexpr (std::is_signed_v<T>)
			value = *m_current == '-' ? (T)INT64_MIN : (T)INT64_MAX;
		else
			value = (T)UINT64_MAX;

		auto [ptr, ec] = std::from_chars((const char*)m_current, (const char*)m_next, value);
		m_current = (const unsigned char*)ptr;

		if (ec != std::errc{})
		{
			if (ec == std::errc::invalid_argument)
				return false;

			while ('0' <= *m_current && *m_current <= '9')
				++m_current;
		}

		skipWhiteSpace();
		return true;
	}

	template <>
	bool extract(unsigned char& value);

	template <>
	bool extract(float& value);

	template <>
	bool extract(bool& value);

	template <>
	bool extract(FloatArray& value);

	template <typename T>
	T extract()
	{
		T value;
		if (!extract(value))
			throw NoParseException();

		return value;
	}

	bool skipInt();

	std::u32string extractText(bool isIniFile = false);
	std::u32string extractLyric();
	bool skipLyric();

	void move(size_t count);

	constexpr void resetPosition() { m_position = 0; }
	uint32_t extractPosition();

	const char* getCurrent() { return (const char*)m_current; }

	size_t getLineNumber() const { return m_lineCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }

	std::string_view extractModifierName();
};
