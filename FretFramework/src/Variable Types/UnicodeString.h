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
	void writeToBch(std::fstream& outFile) const;
	void writeToMid(std::fstream& outFile) const;

	static char s_writeBuffer[5];
	static char* s_bufferStart;
	static size_t s_bufferSize;
	std::string toString() const;
	
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

	std::u32string& get() { return m_string; }
	operator std::string() const { return toString(); }
	friend std::ostream& operator<<(std::ostream& outFile, const UnicodeString& str);
	std::u32string* operator->() const { return (std::u32string* const)&m_string; }
	char32_t& operator[](size_t i) { return m_string[i]; }
};

