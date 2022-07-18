#pragma once
#include <string>
#include <stdexcept>
#include <fstream>

class UnicodeString
{
	class InvalidCharacterException : public std::runtime_error
	{
	public:
		InvalidCharacterException(char32_t value);
	};

	std::u32string m_string;
	std::u32string m_string_lowercase;
	std::u32string m_string_uppercase;

	void setCasedStrings()
	{
		m_string_lowercase = m_string;
		m_string_uppercase = m_string;
		for (size_t i = 0; i < m_string.size(); ++i)
		{
			if (65 <= m_string[i] && m_string[i] <= 90)
				m_string_lowercase[i] += 32;
			else if (97 <= m_string[i] && m_string[i] <= 122)
				m_string_uppercase[i] -= 32;
		}
	}

public:
	UnicodeString() = default;
	UnicodeString(const UnicodeString&) = default;
	UnicodeString(UnicodeString&&) = default;
	UnicodeString(const unsigned char* dataPtr, const unsigned char* const endPtr, bool loadAsLyric = false);
	UnicodeString(const std::string& str);
	UnicodeString(const char32_t* str);
	UnicodeString(const std::u32string& str);
	UnicodeString& assign(const unsigned char* dataPtr, const unsigned char* const endPtr, bool loadAsLyric = false);
	UnicodeString& operator=(const std::u32string& str);
	UnicodeString& operator=(const std::string& str);
	UnicodeString& operator=(const UnicodeString&) = default;
	UnicodeString& operator=(UnicodeString&&) = default;
	void writeToFile(std::fstream& outFile) const;

	static char s_writeBuffer[5];
	static char* s_bufferStart;
	static size_t s_bufferSize;
	std::string toString() const;
	bool empty() const { return m_string.empty(); }
	std::u32string& get() { return m_string; }
	const std::u32string& get() const { return m_string; }

	std::u32string& getLowerCase() { return m_string_lowercase; }
	const std::u32string& getLowerCase() const { return m_string_lowercase; }

	std::u32string& getUpperCase() { return m_string_uppercase; }
	const std::u32string& getUpperCase() const { return m_string_uppercase; }

	bool operator==(const char32_t* str) const
	{
		return m_string == str;
	}
	
	bool operator==(const char* str) const
	{
		size_t size = strlen(str);
		if (m_string.size() != size)
			return false;

		for (size_t i = 0; i < size; ++i)
			if (m_string[i] != str[i])
				return false;
		return true;
	}

	// Comapres by the lowercase versions of the string
	bool operator==(const UnicodeString& str) const
	{
		return m_string_lowercase == str.m_string_lowercase;
	}

	// Comapres by the lowercase versions of the string
	auto operator<=>(const UnicodeString& str) const
	{
		return m_string_lowercase <=> str.m_string_lowercase;
	}

	int compare(const UnicodeString& str) const;
	
	operator std::string() const { return toString(); }
	friend std::ostream& operator<<(std::ostream& outFile, const UnicodeString& str);
	std::u32string* operator->() const { return (std::u32string* const)&m_string; }

	char32_t& operator[](size_t i) { return m_string[i]; }
	const char32_t& operator[](size_t i) const { return m_string[i]; }
};

