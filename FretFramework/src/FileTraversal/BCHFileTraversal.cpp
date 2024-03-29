#include "BCHFileTraversal.h"

BCHTraversal::BCHTraversal(const FilePointers& file)
	: Traversal(file)
{
	if (!validateChunk("BCHF"))
		throw InvalidChunkTagException("BCHF");
}

bool BCHTraversal::validateChunk(const char(&str)[5])
{
	const uint32_t value = ptrToUint32(str);
	if (ptrToUint32(m_current) == value)
	{
		m_current += 4;
		uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(m_current);
		m_current += 4;
		m_nextTrack = m_current + chunkSize;

		if (m_nextTrack > m_end)
			m_nextTrack = m_end;

		if (value == ptrToUint32("BCHF"))
			m_next = m_current + chunkSize;
		else if (value == ptrToUint32("DIFF"))
		{
			m_trackID = *m_current++;
			m_next = m_current + 4;
		}
		else if (value == ptrToUint32("INST"))
		{
			m_trackID = *m_current++;
			m_next = m_current + 1;
		}
		else if (value == ptrToUint32("VOCL"))
		{
			m_trackID = *m_current++;
			m_next = m_current;
		}
		else if (value == ptrToUint32("LYRC"))
			m_next = m_current + 5;
		else
			m_next = m_current + 4;

		if (m_next > m_nextTrack)
			m_next = m_nextTrack;
		
		m_eventCount = 0;
		m_tickPosition = 0;
		return true;
	}
	return false;
}

bool BCHTraversal::checkNextChunk(const char(&str)[5]) const
{
	return ptrToUint32(m_nextTrack) == ptrToUint32(str);
}

const unsigned char* BCHTraversal::findNextChunk(const char(&str)[5]) const
{
	const uint32_t value = ptrToUint32(str);
	const char* test = (const char*)memchr(m_current, str[0], m_end - m_current);
	while (test && ptrToUint32(test) != value)
	{
		++test;
		test = (const char*)memchr(test, str[0], (const char*)m_end - test);
	}
	return (const unsigned char*)test;
}

bool BCHTraversal::doesNextTrackExist()
{
	return m_nextTrack != m_end;
}

void BCHTraversal::setNextTrack(const unsigned char* location)
{
	m_nextTrack = location;
	if (!m_nextTrack)
		m_nextTrack = m_end;
}

bool BCHTraversal::next()
{
	m_current = m_next;
	if (m_current < m_nextTrack)
	{
		++m_eventCount;
		m_tickPosition += WebType::read(m_current);
		m_eventType = *m_current++;
		const uint32_t length = WebType::read(m_current);
		m_next = m_current + length;

		if (m_next > m_nextTrack)
			m_next = m_nextTrack;
		return true;
	}
	else if (m_current > m_nextTrack)
		m_current = m_nextTrack;
	return false;
}

void BCHTraversal::move(size_t count)
{
	m_current += count;
	if (m_current > m_next)
		m_current = m_next;
}

void BCHTraversal::skipTrack()
{
	m_current = m_nextTrack;
	m_next = m_current;
}

std::u32string BCHTraversal::extractText()
{
	if (m_current > m_next)
		throw NoParseException();

	const std::u32string str = UnicodeString::bufferToU32(m_current, m_next - m_current);
	m_current = m_next;
	return str;
}

std::u32string BCHTraversal::extractLyric(uint32_t length)
{
	if (m_current > m_next)
		throw NoParseException();

	if (m_current + length > m_next)
		length = uint32_t(m_next - m_current);

	const std::u32string str = UnicodeString::bufferToU32(m_current, length);
	m_current += length;
	return str;
}
