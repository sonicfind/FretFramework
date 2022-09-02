#pragma once
#include "FileTraversal.h"
#include "Variable Types/UnicodeString.h"

class TextTraversal;

class TxtFileModifier
{
protected:
	const std::string_view m_name;

public:
	constexpr TxtFileModifier(const std::string_view name) : m_name(name) {}
	constexpr virtual ~TxtFileModifier() = default;

	constexpr std::string_view getName() const { return m_name; }

	virtual void read(TextTraversal& traversal) = 0;
	virtual void write(std::fstream& outFile) const = 0;
	virtual void write_ini(std::fstream& outFile) const = 0;
};

class StringModifier;
class StringModifier_Chart;
template <typename T>
class NumberModifier;
class FloatArrayModifier;
class BooleanModifier;

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

	unsigned char extractChar();
	bool extract(unsigned char& value);

	void skipWhiteSpace();

private:
	void skipEqualsSign();

	template <typename T>
	bool try_parseInt(T& value)
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

public:

	template <typename T>
	T extractInt()
	{
		static_assert(std::is_integral_v<T>);

		T value;
		if (!try_parseInt(value))
			throw NoParseException();

		return value;
	}

	template <typename T>
	bool extract(T& value)
	{
		return try_parseInt(value);
	}

	template <>
	bool extract(float& value);

	bool extractBoolean();

	void move(size_t count);

	constexpr void resetPosition() { m_position = 0; }
	uint32_t extractPosition();

	const std::string_view extractModifierName();
	
	template <typename T>
	std::unique_ptr<TxtFileModifier> extractModifier(const T& _MODIFIERLIST)
	{
		const auto modifierName = extractModifierName();
		auto pairIter = std::lower_bound(std::begin(_MODIFIERLIST), std::end(_MODIFIERLIST), modifierName,
			[](const std::pair<std::string_view, std::unique_ptr<TxtFileModifier>(*)()>& pair, const std::string_view str)
			{
				return pair.first < str;
			});

		if (pairIter == std::end(_MODIFIERLIST) || modifierName != pairIter->first)
			return nullptr;

		return pairIter->second();
	}

	std::u32string extractText(bool isIniFile = false);
	std::u32string extractLyric();
	const char* getCurrent() { return (const char*)m_current; }

	size_t getLineNumber() const { return m_lineCount; }

	bool operator==(char c) const { return *m_current == c; }
	bool operator!=(char c) const { return *m_current != c; }
};
