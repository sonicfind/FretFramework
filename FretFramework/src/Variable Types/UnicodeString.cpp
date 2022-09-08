#include "UnicodeString.h"
#include "utf_utils.h"

UnicodeString::InvalidCharacterException::InvalidCharacterException(char32_t value)
	: std::runtime_error("Character value in a u32string cannot exceed 1114111 (value: " + std::to_string(value) + ")") {}

UnicodeString::UnicodeString(const char32_t* str) : m_string(str) { setCasedStrings(); }
UnicodeString::UnicodeString(std::u32string&& str) : m_string(std::move(str)) { setCasedStrings(); }

UnicodeString& UnicodeString::assign(const unsigned char* dataPtr, size_t length)
{
	m_string = bufferToU32(dataPtr, length);
	setCasedStrings();
	return *this;
}

UnicodeString& UnicodeString::operator=(const std::u32string& str)
{
	m_string = str;
	setCasedStrings();
	return *this;
}

constexpr void UnicodeString::setCasedStrings() noexcept
{
	m_string_lowercase = m_string_uppercase = m_string;
	for (size_t i = 0; i < m_string_uppercase.size(); ++i)
	{
		const char32_t character = m_string_uppercase[i];
		if (character < 128)
		{
			m_string_lowercase[i] = s_LOWER[character];
			m_string_uppercase[i] = s_UPPER[character];
		}
	}
}

#include "WebType.h"
void UnicodeString::writeToBCH(std::fstream& outFile) const
{
	U32ToBCH(m_string, outFile);
}

char UnicodeString::s_writeBuffer[5] = {};
char* UnicodeString::s_bufferStart;
size_t UnicodeString::s_bufferSize;
std::string UnicodeString::toString() const
{
	return U32ToStr(m_string);
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

std::u32string UnicodeString::bufferToU32(const unsigned char* dataPtr, size_t length)
{
	std::u32string str(length + 3, 0);
	size_t finalLength = uu::UtfUtils::SseBigTableConvert(dataPtr, dataPtr + length, str.data());
	str.resize(finalLength);
	return str;
}

std::u32string UnicodeString::strToU32(const std::string& str)
{
	return bufferToU32((const unsigned char*)str.c_str(), str.size());
}

std::string UnicodeString::U32ToStr(const std::u32string& u32)
{
	std::string str(u32.size() * 4, 0);
	unsigned char* current = (unsigned char*)str.data();
	for (const char32_t cpt : u32)
		if (uu::UtfUtils::GetCodeUnits(cpt, current) == 0)
			*current++ = '_';
	str.resize((char*)current - str.data());
	return str;
}

void UnicodeString::U32ToBCH(const std::u32string& u32, std::fstream& outFile)
{
	const std::string str = U32ToStr(u32);
	WebType((uint32_t)str.size()).writeToFile(outFile);
	outFile.write(str.data(), str.size());
}
