#include "UnicodeString.h"

UnicodeString::InvalidCharacterException::InvalidCharacterException(char32_t value)
	: std::runtime_error("Character value in a u32string cannot exceed 1114111 (value: " + std::to_string(value) + ")") {}

UnicodeString::UnicodeString(const unsigned char* dataPtr, const unsigned char* const endPtr)
{
	assign(dataPtr, endPtr);
}

UnicodeString::UnicodeString(const std::string& str)
{
	operator=(str);
}

UnicodeString::UnicodeString(const char32_t* str)
	: m_string(str)
{
}

UnicodeString::UnicodeString(const std::u32string& str)
{
	operator=(str);
}

UnicodeString& UnicodeString::assign(const unsigned char* dataPtr, const unsigned char* const endPtr)
{
	m_string.clear();
	while (dataPtr < endPtr)
	{
		char32_t character = 0;
		int numCharacters = 0;
		if (*dataPtr < 0b10000000)
		{
			numCharacters = 1;
			character = *dataPtr;
		}
		else if (*dataPtr < 0b11100000)
		{
			numCharacters = 2;
			character = *dataPtr & 31;
		}
		else if (*dataPtr < 0b11110000)
		{
			numCharacters = 3;
			character = *dataPtr & 15;
		}
		else
		{
			numCharacters = 4;
			character = *dataPtr & 7;
		}

		++dataPtr;
		int c = 1;
		while (dataPtr < endPtr && 128 <= *dataPtr && *dataPtr < 192)
		{
			if (c < numCharacters)
			{
				character <<= 6;
				character |= *dataPtr & 63;
				++c;
			}
			++dataPtr;
		}

		m_string += character;
	}
	return *this;
}

UnicodeString& UnicodeString::operator=(const std::string& str)
{
	m_string.clear();
	for (size_t i = 0; i < str.size();)
	{
		char32_t character = 0;
		int numCharacters = 0;
		if (str[i] > 0)
		{
			numCharacters = 1;
			character = str[i];
		}
		else if (str[i] < -32)
		{
			numCharacters = 2;
			character = str[i] & 31;
		}
		else if (str[i] < -16)
		{
			numCharacters = 3;
			character = str[i] & 15;
		}
		else
		{
			numCharacters = 4;
			character = str[i] & 7;
		}

		++i;
		int index = 1;
		while (i < str.size() && -128 <= str[i] && str[i] < -64)
		{
			if (index < numCharacters)
			{
				character <<= 6;
				character |= str[i] & 63;
				++index;
			}
			++i;
		}

		m_string += character;
	}
	return *this;
}

UnicodeString& UnicodeString::operator=(const std::u32string& str)
{
	for (char32_t c : str)
		if (c > 1114111)
			throw InvalidCharacterException(c);

	m_string = str;
	return *this;
}


void UnicodeString::setCasedStrings()
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

#include "WebType.h"
void UnicodeString::writeToFile(std::fstream& outFile) const
{
	const std::string str = toString();
	WebType((uint32_t)str.size()).writeToFile(outFile);
	outFile.write(str.c_str(), str.size());
}

char UnicodeString::s_writeBuffer[5] = {};
char* UnicodeString::s_bufferStart;
size_t UnicodeString::s_bufferSize;
std::string UnicodeString::toString() const
{
	std::string str;
	for (char32_t character : m_string)
	{
		if (character < 0x80)
			str += static_cast<char>(character);
		else
		{
			s_bufferStart = s_writeBuffer + 4;
			s_bufferSize = 0;

			do
			{
				*--s_bufferStart = (character & 63) | 0x80;
				++s_bufferSize;
				character >>= 6;
			} while (character > 0);

			if (s_bufferSize == 2)
				*s_bufferStart |= 192;
			else if (s_bufferSize == 3)
				*s_bufferStart |= 224;
			else
				*s_bufferStart |= 240;

			str += s_bufferStart;
		}
	}
	return str;
}

std::ostream& operator<<(std::ostream& outFile, const UnicodeString& str)
{
	return outFile << str.toString();
}

bool UnicodeString::operator==(const UnicodeString& str) const
{
	return m_string_lowercase == str.m_string_lowercase;
}

bool UnicodeString::operator<(const UnicodeString& str) const
{
	return m_string_lowercase < str.m_string_lowercase;
}

int UnicodeString::compare(const UnicodeString& str) const
{
	return m_string_lowercase.compare(str.m_string_lowercase);
}
