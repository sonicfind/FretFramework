#include "TextFileTraversal.h"
#include "Variable Types/UnicodeString.h"

std::u32string TextTraversal::extractText(bool isIniFile)
{
	std::u32string str;
	if (!isIniFile && *m_current == '\"')
	{
		const unsigned char* test = m_next - 1;
		while (test > m_current)
		{
			if (*test == '\"' && *(test - 1) != '\\')
			{
				++m_current;
				str = UnicodeString::bufferToU32(m_current, test - m_current);
				m_current = test + 1;
				skipWhiteSpace();
				goto RemoveSlashes;
			}
			else if (*test == '\"')
				test -= 2;
			else
				--test;
		}
	}

	if (m_current == m_next)
		return str;

	{
		size_t length = m_next - m_current - 1;
		if (m_current[length] != '\r')
			++length;
		str = UnicodeString::bufferToU32(m_current, length);
		m_current = m_next;
	}

RemoveSlashes:
	for (size_t pos = str.find(U"\\\""); pos != std::string::npos; pos = str.find(U"\\\"", pos))
		str.erase(pos);

	size_t pos = str.size();
	while (pos > 0 && (str[pos - 1] == ' ' || str[pos - 1] == '\t'))
		--pos;

	str.resize(pos);
	return str;
}

std::u32string TextTraversal::extractLyric()
{
	if (*m_current == '\"')
	{
		const unsigned char* test = m_current + 1;
		do
			test = std::find(test, m_next, '\"');
		while (test && test[-1] == '\\' && test < m_next);

		if (test != nullptr && test < m_next)
		{
			++m_current;
			std::u32string str = UnicodeString::bufferToU32(m_current, test - m_current);
			m_current = test + 1;
			skipWhiteSpace();

			for (size_t pos = str.find(U"\\\""); pos != std::string::npos; pos = str.find(U"\\\"", pos))
				str.erase(pos);

			return str;
		}
	}
	throw InvalidLyricExcpetion();
}

bool TextTraversal::skipLyric()
{
	if (*m_current == '\"')
	{
		const unsigned char* test = m_current + 1;
		do
			test = std::find(test, m_next, '\"');
		while (test && test[-1] == '\\' && test < m_next);

		if (test != nullptr && test < m_next)
		{
			m_current = test + 1;
			skipWhiteSpace();
			return true;
		}
	}
	return false;
}

