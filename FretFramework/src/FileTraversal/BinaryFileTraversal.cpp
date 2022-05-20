#include "BinaryFileTraversal.h"

BinaryTraversal::BinaryTraversal(const std::filesystem::path& path)
	: Traversal(path)
	, m_nextTrack(m_current) {}

bool BinaryTraversal::validateChunk(const char(&str)[5])
{
	if (strncmp((const char*)m_current, str, 4) == 0)
	{
		m_current += 4;
		uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(m_current);
		m_current += 4;
		m_nextTrack = m_current + chunkSize;

		if (strncmp(str, "BCHF", 4) == 0)
			m_next = m_current + chunkSize;
		else
		{
			if (strncmp(str, "INST", 4) == 0)
				m_next = m_current + 2;
			else if (strncmp(str, "DIFF", 4) == 0)
				m_next = m_current + 5;
			else
				m_next = m_current + 4;
		}
		
		m_eventCount = 0;
		m_tickPosition = 0;
		return true;
	}
	return false;
}

bool BinaryTraversal::checkNextChunk(const char(&str)[5]) const
{
	return strncmp((const char*)m_nextTrack, str, 4) == 0;
}

const unsigned char* BinaryTraversal::findNextChunk(const char(&str)[5]) const
{
	return (const unsigned char*)strstr((const char*)m_current, str);
}

bool BinaryTraversal::doesNextTrackExist()
{
	return m_nextTrack < m_end;
}

void BinaryTraversal::setNextTrack(const unsigned char* location)
{
	if (location)
		m_nextTrack = location;
	else
		m_nextTrack = m_end;
}

bool BinaryTraversal::next()
{
	m_current = m_next;
	if (m_current < m_nextTrack)
	{
		++m_eventCount;
		m_tickPosition += WebType(m_current);
		m_eventType = *m_current++;
		m_next = m_current + WebType(m_current);
		return true;
	}
	else if (m_current > m_nextTrack)
		m_current = m_nextTrack;
	return false;
}

void BinaryTraversal::move(size_t count)
{
	if (m_current + count <= m_next)
		m_current += count;
	else
		m_current = m_next;
}

void BinaryTraversal::skipTrack()
{
	m_current = m_nextTrack;
}

std::string BinaryTraversal::extractText()
{
	std::string str((const char*)m_current, m_next - m_current);
	m_current = m_next;
	return str;
}

std::string BinaryTraversal::extractText(uint32_t length)
{
	if (m_current + length > m_next)
		length = uint32_t(m_next - m_current);

	std::string str((const char*)m_current, length);
	m_current += length;
	return str;
}

bool BinaryTraversal::extract(uint32_t& value)
{
	if (m_current + 4 <= m_next)
	{
		value = *reinterpret_cast<const uint32_t*>(m_current);
		m_current += 4;
		return true;
	}
	return false;
}

bool BinaryTraversal::extract(uint16_t& value)
{
	if (m_current + 2 <= m_next)
	{
		value = *reinterpret_cast<const uint16_t*>(m_current);
		m_current += 2;
		return true;
	}
	return false;
}

bool BinaryTraversal::extract(unsigned char& value)
{
	if (m_current < m_next)
	{
		value = *m_current++;
		return true;
	}
	return false;
}

bool BinaryTraversal::extractVarType(uint32_t& value)
{
	value = WebType(m_current);
	return m_current <= m_next;
}

uint32_t BinaryTraversal::extractVarType()
{
	WebType value(m_current);
	if (m_current > m_next)
		throw NoParseException();
	return value;
}

BinaryTraversal::operator uint32_t()
{ 
	if (m_current + 4 > m_next)
		throw NoParseException();

	uint32_t val = *reinterpret_cast<const uint32_t*>(m_current);
	m_current += 4;
	return val;
}

BinaryTraversal::operator uint16_t()
{ 
	if (m_current + 2 > m_next)
		throw NoParseException();

	uint16_t val = *reinterpret_cast<const uint16_t*>(m_current);
	m_current += 2;
	return val;
}

unsigned char BinaryTraversal::extract()
{
	if (m_current == m_next)
		throw NoParseException();

	return *m_current++;
}
