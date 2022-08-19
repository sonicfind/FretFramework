#include "TextFileTraversal.h"

void TextTraversal::skipWhiteSpace(const unsigned char*& curr)
{
	while (*curr == ' ' || *curr == '\t')
		++curr;
}

void TextTraversal::skipEqualsSign(const unsigned char*& curr)
{
	while (*curr == '=')
		++curr;
	skipWhiteSpace(curr);
}

TextTraversal::TextTraversal(const FilePointers& file)
	: Traversal(file)
{
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0};
	if (strncmp((const char*)m_current, BOM, 3) == 0)
		m_current += 3;

	if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
		m_next = m_end;

	skipWhiteSpace(m_current);
	if (*m_current == '[')
		setTrackName();
}

bool TextTraversal::next()
{
	do
	{
		m_current = m_next;
		if (m_current >= m_end)
			return false;

		++m_lineCount;
		++m_current;

		if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
			m_next = m_end;

		skipWhiteSpace(m_current);
	} while (*m_current == '\n');
		
	if (*m_current == '[')
		setTrackName();
	return true;
}

void TextTraversal::skipTrack()
{
	int scopeTracker = 0;
	do
	{
		if (*m_current == '[' &&
			(scopeTracker == 0 || !next()))
			return;
		
		while (*m_current == '{')
		{
			if (!next())
				return;
			++scopeTracker;
		}

		while (*m_current == '}')
		{
			if (!next() || scopeTracker <= 1)
				return;
			--scopeTracker;
		}

		auto getLineWithCharacter = [&](const char stopCharacter)
		{
			const char* position = (const char*)m_next;
			while (position = strchr(position, stopCharacter))
			{
				const char* test = position - 1;
				while (*test == ' ' || *test == '\t')
					--test;

				if (*test == '\n')
				{
					position = test;
					break;
				}
				else
					++position;
			}
			return (const unsigned char*)position;
		};

		const unsigned char* const openBracket = getLineWithCharacter('[');
		const unsigned char* const openBrace = getLineWithCharacter('{');
		const unsigned char* const closeBrace = getLineWithCharacter('}');

		if (openBracket && (!openBrace || openBracket < openBrace) && (!closeBrace || openBracket < closeBrace))
			m_next = openBracket;
		else if (openBrace && (!closeBrace || openBrace < closeBrace))
			m_next = openBrace;
		else if (closeBrace)
			m_next = closeBrace;
		else
			m_next = m_end;
	} while (next());
}

void TextTraversal::move(size_t count)
{
	m_current += count;
	skipWhiteSpace(m_current);
}

void TextTraversal::setTrackName()
{
	const unsigned char* loc = (const unsigned char*)strchr((const char*)m_current, ']');
	if (loc == nullptr || (const unsigned char*)loc > m_next)
		loc = m_next;
	else
		++loc;

	m_trackName = std::string_view((const char*)m_current, loc - m_current);
}

bool TextTraversal::isTrackName(const char* str) const
{
	return m_trackName == str;
}

bool TextTraversal::cmpTrackName(const std::string_view& str)
{
	if (m_trackName.starts_with(str))
	{
		m_trackName.remove_prefix(str.length());
		return true;
	}
	return false;
}

std::string TextTraversal::getTrackName() const
{
	std::string track;
	for (const char c : m_trackName)
		track += std::tolower(c);
	return track;
}

uint32_t TextTraversal::extractPosition()
{
	uint32_t nextPosition = extractInt<uint32_t>();

	if (m_position <= nextPosition)
	{
		skipEqualsSign(m_current);
		m_position = nextPosition;
		return m_position;
	}
	throw "position out of order (previous:  " + std::to_string(m_position) + ')';
}

bool TextTraversal::cmpModifierName(const std::string_view& name)
{
	const size_t length = name.length();
	if (strncmp((const char*)m_current, name.data(), length) == 0 &&
		(m_current[length] == ' ' || m_current[length] == '='))
	{
		m_current += length;
		skipEqualsSign(m_current);
		return true;
	}
	return false;
}

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
				skipWhiteSpace(m_current);
				goto RemoveSlashes;
			}
			else if (*test == '\"')
				test -= 2;
			else
				--test;
		}
	}

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
		const unsigned char* test = (const unsigned char*)strchr((const char*)m_current + 1, '\"');

		while (test && *(test - 1) == '\\' && test < m_next)
			test = (const unsigned char*)strchr((const char*)test, '\"');

		if (test != nullptr && test < m_next)
		{
			++m_current;
			std::u32string str = UnicodeString::bufferToU32(m_current, test - m_current);
			m_current = test + 1;
			skipWhiteSpace(m_current);

			for (size_t pos = str.find(U"\\\""); pos != std::string::npos; pos = str.find(U"\\\"", pos))
				str.erase(pos);

			return str;
		}
	}
	throw InvalidLyricExcpetion();
}

unsigned char TextTraversal::extractChar()
{
	if (m_current >= m_next)
		throw NoParseException();

	const unsigned char c = *m_current++;
	skipWhiteSpace(m_current);
	return c;
}

bool TextTraversal::extract(unsigned char& value)
{
	if (m_current >= m_next)
		return false;

	value = *m_current++;
	skipWhiteSpace(m_current);
	return true;
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
		std::u32string str = extractText();
		value = str.length() >= 4 &&
			(str[0] == 't' || str[0] == 'T') &&
			(str[1] == 'r' || str[1] == 'R') &&
			(str[2] == 'u' || str[2] == 'U') &&
			(str[3] == 'e' || str[3] == 'E');

		break;
	}
}

void TextTraversal::extract(UnicodeString& str)
{
	str = std::move(extractText());
}

void TextTraversal::extract(float(&arr)[2])
{
	extract(arr[0]);
	extract(arr[1]);
}
