#include "TextFileTraversal.h"

void TextTraversal::skipWhiteSpace()
{
	while (*m_current <= 32 && *m_current != '\n')
		++m_current;
}

void TextTraversal::skipEqualsSign()
{
	while (*m_current == '=')
		++m_current;
	skipWhiteSpace();
}

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

bool TextTraversal::next()
{
	do
	{
		m_current = m_next;
		++m_current;
		if (m_current >= m_end)
			return false;

		++m_lineCount;
		

		if (!(m_next = (const unsigned char*)strchr((const char*)m_current, '\n')))
			m_next = m_end;

		skipWhiteSpace();
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

uint32_t TextTraversal::extractPosition()
{
	const uint32_t prevPosition = m_position;
	m_position = extractInt<uint32_t>();

	if (m_position < prevPosition)
	{
		m_position = prevPosition;
		throw "position out of order (previous:  " + std::to_string(m_position) + ')';
	}

	skipEqualsSign();
	return m_position;
}

const std::string_view TextTraversal::extractModifierName()
{
	const char* const start = (const char*)m_current;
	while (m_current < m_next &&
		*m_current != ' ' &&
		*m_current != '\t' &&
		*m_current != '=')
		++m_current;

	const std::string_view modifierName(start, (const char*)m_current);
	skipWhiteSpace();
	skipEqualsSign();
	return modifierName;
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
	const char* const cNext = reinterpret_cast<const char*>(m_next);
	if (*m_current == '\"')
	{
		const char* test = reinterpret_cast<const char*>(m_current + 1);
		do 
			test = strchr(test, '\"');
		while (test && test[-1] == '\\' && test < cNext);

		if (test != nullptr && test < cNext)
		{
			const unsigned char* const end = reinterpret_cast<const unsigned char*>(test);
			++m_current;
			std::u32string str = UnicodeString::bufferToU32(m_current, end - m_current);
			m_current = end + 1;
			skipWhiteSpace();

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

float TextTraversal::extractFloat()
{
	float value = 0;
	auto [ptr, ec] = std::from_chars((const char*)m_current, (const char*)m_next, value);
	m_current = (const unsigned char*)ptr;

	if (ec != std::errc{} && ec != std::errc::invalid_argument)
		while (('0' <= *m_current && *m_current <= '9') || *m_current == '.')
			++m_current;
	return value;
}

bool TextTraversal::extractBoolean()
{
	switch (*m_current)
	{
	case '0':
		return false;
	case '1':
		return true;
	default:
		const std::u32string str = extractText();
		return str.length() >= 4 &&
			(str[0] == 't' || str[0] == 'T') &&
			(str[1] == 'r' || str[1] == 'R') &&
			(str[2] == 'u' || str[2] == 'U') &&
			(str[3] == 'e' || str[3] == 'E');

		break;
	}
}
