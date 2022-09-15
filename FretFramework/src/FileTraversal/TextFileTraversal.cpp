#include "TextFileTraversal.h"

void TextTraversal::skipWhiteSpace()
{
	static constexpr bool VALIDS[256] =
	{
		false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,
		true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
		true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, true,  false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	};

	while (VALIDS[*m_current])
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
	return true;
}

void TextTraversal::skipTrack()
{
	if (*m_current != '[')
	{
		const char* openBracket = (const char*)m_next;
		while (openBracket = strchr(openBracket, '['))
		{
			const char* test = openBracket - 1;
			while (*test == ' ' || *test == '\t')
				--test;

			if (*test == '\n')
			{
				m_next = (const unsigned char*)test;
				return;
			}
			else
				++openBracket;
		}

		m_next = m_end;
	}
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

template <>
bool TextTraversal::extract(unsigned char& value)
{
	if (m_current >= m_next)
		return false;

	value = *m_current++;
	skipWhiteSpace();
	return true;
}

template<>
bool TextTraversal::extract(bool& value)
{
	switch (*m_current)
	{
	case '0':
		value = false; break;
	case '1':
		value = true; break;
	default:
		value = m_current + 4 <= m_next &&
			(m_current[0] == 't' || m_current[0] == 'T') &&
			(m_current[1] == 'r' || m_current[1] == 'R') &&
			(m_current[2] == 'u' || m_current[2] == 'U') &&
			(m_current[3] == 'e' || m_current[3] == 'E');
	}
	return true;
}

template <>
bool TextTraversal::extract(float& value)
{
	auto [ptr, ec] = std::from_chars((const char*)m_current, (const char*)m_next, value);
	m_current = (const unsigned char*)ptr;

	if (ec != std::errc{})
	{
		if (ec == std::errc::invalid_argument)
			return false;

		while (('0' <= *m_current && *m_current <= '9') || *m_current == '.')
			++m_current;
	}

	return true;
}

template <>
bool TextTraversal::extract(FloatArray& value)
{
	return extract(value[0]) && extract(value[1]);
}

bool TextTraversal::skipInt()
{
	const unsigned char* const start = m_current;
	while ('0' <= *m_current && *m_current <= '9')
		++m_current;

	return m_current != start;
}
