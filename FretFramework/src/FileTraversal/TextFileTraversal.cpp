#include "TextFileTraversal.h"

void TextTraversal::skipWhiteSpace()
{
	auto check = [&]
	{
		if (*m_current <= 32)
			return *m_current != '\n';
		else
			return *m_current == '=';
	};

	while (check())
		++m_current;
}

TextTraversal::TextTraversal(const FilePointers& file)
	: Traversal(file)
{
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0};
	if (strncmp((const char*)m_current, BOM, 3) == 0)
		m_current += 3;

	m_next = std::find(m_current, m_end, '\n');

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
		
		m_next = std::find(m_current, m_end, '\n');

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
			const unsigned char* position = m_next;
			while ((position = std::find(position, m_end, stopCharacter)) < m_end)
			{
				const unsigned char* test = position - 1;
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
			return position;
		};

		const unsigned char* const openBracket = getLineWithCharacter('[');
		const unsigned char* const openBrace = getLineWithCharacter('{');
		const unsigned char* const closeBrace = getLineWithCharacter('}');

		if (openBracket < openBrace && openBracket < closeBrace)
			m_next = openBracket;
		else if (openBrace < closeBrace)
			m_next = openBrace;
		else
			m_next = closeBrace;
	} while (next());
}

void TextTraversal::move(size_t count)
{
	m_current += count;
	skipWhiteSpace();
}

void TextTraversal::setTrackName()
{
	const unsigned char* loc = std::find(m_current, m_end, ']');
	++loc;
	if (loc > m_next)
		loc = m_next;

	m_trackName = std::string_view((const char*)m_current, loc - m_current);
}

bool TextTraversal::isTrackName(const char* str) const
{
	return m_trackName == str;
}

bool TextTraversal::cmpTrackName(const std::string_view str)
{
	if (m_trackName.starts_with(str))
	{
		m_trackName.remove_prefix(str.length());
		return true;
	}
	return false;
}

std::string TextTraversal::getLowercaseTrackName() const
{
	std::string track(m_trackName);
	for (char& c : track)
		if (65 <= c && c <= 90)
			c += 32;
	return track;
}

uint32_t TextTraversal::extractPosition()
{
	const uint32_t prevPosition = m_position;
	m_position = extract<uint32_t>();

	if (m_position < prevPosition)
	{
		m_position = prevPosition;
		throw "position out of order (previous:  " + std::to_string(m_position) + ')';
	}

	return m_position;
}

std::string_view TextTraversal::extractModifierName()
{
	const char* const start = (const char*)m_current;
	while (*m_current != ' ' &&
		*m_current != '\t' &&
		*m_current != '=')
		++m_current;

	const std::string_view modifierName(start, (const char*)m_current);
	skipWhiteSpace();
	return modifierName;
}


template<>
unsigned char TextTraversal::extract()
{
	if (m_current >= m_next)
		throw NoParseException();

	const unsigned char c = *m_current++;
	skipWhiteSpace();
	return c;
}

template<>
bool TextTraversal::extract()
{
	switch (*m_current)
	{
	case '0':
		return false;
	case '1':
		return true;
	default:
		return m_current + 4 <= m_next &&
			(m_current[0] == 't' || m_current[0] == 'T') &&
			(m_current[1] == 'r' || m_current[1] == 'R') &&
			(m_current[2] == 'u' || m_current[2] == 'U') &&
			(m_current[3] == 'e' || m_current[3] == 'E');

		break;
	}
}

template <>
float TextTraversal::extract()
{
	float value = 0;
	auto [ptr, ec] = std::from_chars((const char*)m_current, (const char*)m_next, value);
	m_current = (const unsigned char*)ptr;

	if (ec != std::errc{})
	{
		if (ec == std::errc::invalid_argument)
			throw NoParseException();

		while (('0' <= *m_current && *m_current <= '9') || *m_current == '.')
			++m_current;
	}

	return value;
}

template <>
FloatArray TextTraversal::extract()
{
	return { extract<float>(), extract<float>() };
}
