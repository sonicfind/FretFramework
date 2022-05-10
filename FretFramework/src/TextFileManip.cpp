#include "TextFileManip.h"

TextTraversal::TextTraversal(const std::filesystem::path& path)
{
	std::fstream inFile = FilestreamCheck::getFileStream(path, std::ios_base::in);
	inFile.seekg(0, inFile.end);
	size_t length = inFile.tellg();
	inFile.seekg(0, inFile.beg);

	m_file = new char[length + 1]();
	m_end = m_file + length;
	inFile.read((char*)m_file, length);
	inFile.close();

	m_current = m_file;
	if (!(m_next = strchr(m_current, '\n')))
		m_next = m_end - 1;
}

void TextTraversal::skipWhiteSpace()
{
	while (m_current < m_end && m_current < m_next)
	{
		switch (*m_current)
		{
		case ' ':
		case '\t':
			++m_current;
			break;
		default:
			return;
		}
	}
}

void TextTraversal::nextLine()
{
	if (m_current < m_end)
	{
		++m_lineCount;
		m_current = m_next + 1;
		if (!(m_next = strchr(m_current, '\n')))
			m_next = m_end - 1;
		skipWhiteSpace();
	}
}

void TextTraversal::skipScope()
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
			nextLine();
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
		move(1);
}

std::string_view TextTraversal::extractText()
{
	if (*m_current == '\"')
	{
		const char* test = m_next - 1;
		while (test > m_current)
		{
			if (*test == '\"')
			{
				std::string_view str(m_current + 1, test - m_current - 1);
				m_current = test + 1;
				skipWhiteSpace();
				return str;
			}
		}
	}

	std::string_view str(m_current, m_next - m_current);
	m_current = m_next;
	return str;
}

bool TextTraversal::extractUInt(uint32_t& value)
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

TextTraversal::~TextTraversal()
{
	delete[m_end - m_file + 1] m_file;
}
