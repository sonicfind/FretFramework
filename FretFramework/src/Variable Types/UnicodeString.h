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

public:
	UnicodeString() = default;
	UnicodeString(const unsigned char* dataPtr, const unsigned char* const endPtr);
	UnicodeString(const std::string& str);
	UnicodeString(const char32_t* str);
	UnicodeString(const std::u32string& str);
	UnicodeString& assign(const unsigned char* dataPtr, const unsigned char* const endPtr);
	UnicodeString& operator=(const std::u32string& str);
	UnicodeString& operator=(const std::string& str);
	UnicodeString& operator=(const UnicodeString& str) = default;
	void writeToFile(std::fstream& outFile) const;

	static char s_writeBuffer[5];
	static char* s_bufferStart;
	static size_t s_bufferSize;
	std::string toString() const;
	std::u32string& get() { return m_string; }
	std::u32string getLowerCase() const
	{
		std::u32string str = m_string;
		for (char32_t c : m_string)
		{
			if (65 <= c && c <= 90)
				str += c + 32;
			else
				str += c;
		}
		return str;
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

	bool operator==(const UnicodeString& str) const
	{
		return getLowerCase() == str.getLowerCase();
	}

	auto operator<=>(const UnicodeString& str) const
	{
		return getLowerCase() <=> str.getLowerCase();
	}
	
	operator std::string() const { return toString(); }
	friend std::ostream& operator<<(std::ostream& outFile, const UnicodeString& str);
	std::u32string* operator->() const { return (std::u32string* const)&m_string; }
	char32_t& operator[](size_t i) { return m_string[i]; }
};

