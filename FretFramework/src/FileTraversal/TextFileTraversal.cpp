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

bool TextTraversal::extract(uint32_t& value)
{
	if ('0' <= *m_current && *m_current <= '9')
	{
		value = *(m_current++) & 15;
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char add = *(m_current++) & 15;
			if (value <= UINT32_MAX / 10)
			{
				value *= 10;
				if (value <= UINT32_MAX - add)
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
	return false;
}

bool TextTraversal::extract(uint16_t& value)
{
	if ('0' <= *m_current && *m_current <= '9')
	{
		value = *(m_current++) & 15;
		while ('0' <= *m_current && *m_current <= '9')
		{
			const char add = *(m_current++) & 15;
			if (value <= UINT16_MAX / 10)
			{
				value *= 10;
				if (value <= UINT16_MAX - add)
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
	return false;
}

unsigned char TextTraversal::extract()
{
	const unsigned char c = *m_current++;
	skipWhiteSpace();
	return c;
}
