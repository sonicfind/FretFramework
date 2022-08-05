#include "TextFileTraversal.h"

TextTraversal::TextTraversal(const FilePointers& file)
	: Traversal(file)
{
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0};
	if (strncmp((const char*)m_current, BOM, 3) == 0)
		m_current += 3;

	if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
		m_next = m_end;

	skipWhiteSpace();
	if (*m_current == '[')
		setTrackName();
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
		if (*m_current == '[')
			setTrackName();
		return true;
	}
	return false;
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
	if (m_current > m_end)
		m_current = m_end;
	else
		skipWhiteSpace();
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

void TextTraversal::skipEqualsSign()
{
	while (*m_current == '=')
		++m_current;
	skipWhiteSpace();
}

uint32_t TextTraversal::extractPosition()
{
	uint32_t nextPosition = extractU32();

	if (m_position <= nextPosition)
	{
		skipEqualsSign();
		m_position = std::move(nextPosition);
		return m_position;
	}
	throw "position out of order (previous:  " + std::to_string(m_position) + ')';
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
				skipWhiteSpace();
				goto RemoveSlashes;
			}
			else if (*test == '\"')
				test -= 2;
			else
				--test;
		}
	}

	// minus 1 to not capture the /r character
	str = UnicodeString::bufferToU32(m_current, m_next - m_current - 1);
	m_current = m_next;

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
			skipWhiteSpace();

			for (size_t pos = str.find(U"\\\""); pos != std::string::npos; pos = str.find(U"\\\"", pos))
				str.erase(pos);

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
