#pragma once
#include "FileTraversal.h"

struct ModifierNode
{
	const std::string_view name;
	const enum Type
	{
		STRING,
		STRING_CHART,
		UINT32,
		INT32,
		UINT16,
		BOOL,
		FLOAT,
		FLOATARRAY
	} type;
};

class TxtFileModifier;

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
	bool next() override;
	void skipTrack() override;

	void setTrackName();
	bool isTrackName(const char* str) const;
	bool cmpTrackName(const std::string_view str);
	std::string getLowercaseTrackName() const;

	void skipWhiteSpace();

private:
	void skipEqualsSign();

	template <typename T>
	bool try_parseInt(T& value)
	{
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

public:

	template <typename T>
	T extract()
	{
		static_assert(std::is_integral_v<T>);

		T value;
		if (!try_parseInt(value))
			throw NoParseException();

		return value;
	}

	template <>
	float extract();

	template <>
	bool extract();

	template <>
	unsigned char extract();

	std::u32string extractText(bool isIniFile = false);
	std::u32string extractLyric();

	template <typename T>
	bool extract(T& value)
	{
		static_assert(std::is_integral_v<T>);
		return try_parseInt(value);
	}

	bool extract(unsigned char& value);

	void move(size_t count);

	constexpr void resetPosition() { m_position = 0; }
	uint32_t extractPosition();

	const char* getCurrent() { return (const char*)m_current; }

	size_t getLineNumber() const { return m_lineCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }



	const std::string_view extractModifierName();
	
	template <size_t SIZE>
	const ModifierNode* testForModifierName(const std::pair<std::string_view, ModifierNode> (&_MODIFIERLIST)[SIZE])
	{
		const auto modifierName = extractModifierName();
		auto pairIter = std::lower_bound(std::begin(_MODIFIERLIST), std::end(_MODIFIERLIST), modifierName,
			[](const std::pair<std::string_view, ModifierNode>& pair, const std::string_view str)
			{
				return pair.first < str;
			});

		if (pairIter == std::end(_MODIFIERLIST) || modifierName != pairIter->first)
			return nullptr;

		return &pairIter->second;
	}

	std::unique_ptr<TxtFileModifier> createModifier(const ModifierNode* node);
};
