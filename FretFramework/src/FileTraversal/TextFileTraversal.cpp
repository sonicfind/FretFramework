#include "TextFileTraversal.h"

TextTraversal::TextTraversal(const std::filesystem::path& path)
	: Traversal(path)
{
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0 };
	if (strstr((const char*)m_current, BOM))
		m_current += 3;

	skipWhiteSpace();
	if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
		m_next = m_end;
}

void TextTraversal::skipWhiteSpace()
{
	while (m_current < m_end && m_current < m_next &&
		(*m_current == ' ' || *m_current == '\t'))
		++m_current;
}

bool TextTraversal::next()
{
	m_current = m_next;
	if (m_current < m_end)
	{
		++m_lineCount;
		++m_current;

		if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
			m_next = m_end;

		skipWhiteSpace();
		return true;
	}
	return false;
}

void TextTraversal::skipTrack()
{
	int scopeTracker = 1;
	while (*this && scopeTracker > 0)
	{
		if (*m_current == '}')
			--scopeTracker;
		else
		{
			if (*m_current == '{')
				++scopeTracker;
			next();
		}
	}
}

void TextTraversal::move(size_t count)
{
	m_current += count;
	if (m_current > m_end)
		m_current = m_end;
	else
		skipWhiteSpace();
}

void TextTraversal::skipEqualsSign()
{
	while (*m_current == '=')
		++m_current;
	skipWhiteSpace();
}

std::string_view TextTraversal::extractText(bool checkForQuotes)
{
	if (checkForQuotes && *m_current == '\"')
	{
		const unsigned char* test = m_next - 1;
		while (test > m_current)
		{
			if (*test == '\"')
			{
				std::string_view str((const char*)m_current + 1, test - m_current - 1);
				m_current = test + 1;
				skipWhiteSpace();
				return str;
			}
			--test;
		}
	}

	// minus to not capture the /r character
	std::string_view str((const char*)m_current, m_next - m_current - 1);
	m_current = m_next;
	return str;
}

std::string TextTraversal::extractLyric()
{
	if (*m_current == '\"')
	{
		const unsigned char* test = (const unsigned char*)strchr((const char*)m_current + 1, '\"');
		if (test != nullptr && test < m_next)
		{
			std::string str((const char*)m_current + 1, test - m_current - 1);
			m_current = test + 1;
			skipWhiteSpace();
			return str;
		}
	}
	throw InvalidLyricExcpetion();
}

uint32_t TextTraversal::extractU32()
{
	if (*m_current < '0' || '9' < *m_current)
		throw NoParseException();

	uint32_t value = *(m_current++) & 15;
	while ('0' <= *m_current && *m_current <= '9')
	{
		const char add = *(m_current++) & 15;
		if (value < UINT32_MAX / 10)
		{
			value *= 10;
			if (value < UINT32_MAX - add)
				value += add;
			else
				value = UINT32_MAX;
		}
		else
			value = UINT32_MAX;
	}
	skipWhiteSpace();
	return value;
}

uint16_t TextTraversal::extractU16()
{
	if (*m_current < '0' || '9' < *m_current)
		throw NoParseException();

	uint16_t value = *(m_current++) & 15;
	while ('0' <= *m_current && *m_current <= '9')
	{
		const char add = *(m_current++) & 15;
		if (value < UINT16_MAX / 10)
		{
			value *= 10;
			if (value < UINT16_MAX - add)
				value += add;
			else
				value = UINT16_MAX;
		}
		else
			value = UINT16_MAX;
	}
	skipWhiteSpace();
	return value;
}

bool TextTraversal::extract(uint32_t& value)
{
	if (*m_current < '0' || '9' < *m_current)
		return false;

	value = *(m_current++) & 15;
	while ('0' <= *m_current && *m_current <= '9')
	{
		const char add = *(m_current++) & 15;
		if (value < UINT32_MAX / 10)
		{
			value *= 10;
			if (value < UINT32_MAX - add)
				value += add;
			else
				value = UINT32_MAX;
		}
		else
			value = UINT32_MAX;
	}
	skipWhiteSpace();
	return true;
}

bool TextTraversal::extract(uint16_t& value)
{
	if (*m_current < '0' || '9' < *m_current)
		return false;

	value = *(m_current++) & 15;
	while ('0' <= *m_current && *m_current <= '9')
	{
		const char add = *(m_current++) & 15;
		if (value < UINT16_MAX / 10)
		{
			value *= 10;
			if (value < UINT16_MAX - add)
				value += add;
			else
				value = UINT16_MAX;
		}
		else
			value = UINT16_MAX;
	}
	skipWhiteSpace();
	return true;
}

unsigned char TextTraversal::extractChar()
{
	if (m_current >= m_next)
		throw NoParseException();

	const unsigned char c = *m_current++;
	skipWhiteSpace();
	return c;
}

bool TextTraversal::extract(unsigned char& value)
{
	if (m_current >= m_next)
		return false;

	value = *m_current++;
	skipWhiteSpace();
	return true;
}

void TextTraversal::extract(int16_t& value)
{
	bool negative = false;
	if (*m_current == '-')
	{
		negative = true;
		++m_current;
	}

	if (*m_current < '0' || '9' < *m_current)
		return;

	value = *(m_current++) & 15;
	if (!negative)
	{
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char add = *(m_current++) & 15;
			if (value < INT16_MAX / 10)
			{
				value *= 10;
				if (value < INT16_MAX - add)
					value += add;
				else
					value = INT16_MAX;
			}
			else
				value = INT16_MAX;
		}
	}
	else
	{
		value *= -1;
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char sub = *(m_current++) & 15;
			if (value > INT16_MIN / 10)
			{
				value *= 10;
				if (value > INT16_MIN + sub)
					value -= sub;
				else
					value = INT16_MIN;
			}
			else
				value = INT16_MIN;
		}
	}
}

void TextTraversal::extract(int32_t& value)
{
	bool negative = false;
	if (*m_current == '-')
	{
		negative = true;
		++m_current;
	}

	if (*m_current < '0' || '9' < *m_current)
		return;

	value = *(m_current++) & 15;
	if (!negative)
	{
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char add = *(m_current++) & 15;
			if (value < INT32_MAX / 10)
			{
				value *= 10;
				if (value < INT32_MAX - add)
					value += add;
				else
					value = INT32_MAX;
			}
			else
				value = INT32_MAX;
		}
	}
	else
	{
		value *= -1;
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char sub = *(m_current++) & 15;
			if (value > INT32_MIN / 10)
			{
				value *= 10;
				if (value > INT32_MIN + sub)
					value -= sub;
				else
					value = INT32_MIN;
			}
			else
				value = INT32_MIN;
		}
	}
}

void TextTraversal::extract(float& value)
{
	unsigned char* end = nullptr;
	value = strtof((const char*)m_current, (char**)&end);
	m_current = end;
}

void TextTraversal::extract(bool& value)
{
	switch (*m_current)
	{
	case '0':
		value = false;
		break;
	case '1':
		value = true;
		break;
	default:
		std::string_view str = extractText();
		value = str.length() >= 4 &&
			(str[0] == 't' || str[0] == 'T') &&
			(str[1] == 'r' || str[1] == 'R') &&
			(str[2] == 'u' || str[2] == 'U') &&
			(str[3] == 'e' || str[3] == 'E');

		break;
	}
}

void TextTraversal::extract(std::string& str)
{
	str = extractText();
}

void TextTraversal::extract(float(&arr)[2])
{
	extract(arr[0]);
	extract(arr[1]);
}
