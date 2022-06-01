#include "MidiFileTraversal.h"
#include "Variable Types/VariableLengthQuantity.h"

MidiTraversal::MidiTraversal(const std::filesystem::path& path)
	: Traversal(path)
	, m_nextTrack(m_current)
{
	if (strncmp((const char*)m_current, "MThd", 4) != 0)
		throw InvalidChunkTagException("MThd");

	m_current += 4;
	uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(m_current);

	if (chunkSize != 6)
		throw "Invalid size of header chunk";\

	m_current += 4;
	m_nextTrack = m_current + chunkSize;

	m_format = *reinterpret_cast<const uint16_t*>(m_current);
	m_current += 2;
	m_numTracks = *reinterpret_cast<const uint16_t*>(m_current) + 1;
	m_current += 2;
	m_tickRate = *reinterpret_cast<const uint16_t*>(m_current);
	m_current += 2;
}

bool MidiTraversal::validateChunk()
{
	if (strncmp((const char*)m_current, "MTrk", 4) == 0)
	{
		++m_trackCount;
		m_current += 4;
		uint32_t chunkSize = *reinterpret_cast<const uint32_t*>(m_current);
		m_current += 4;
		m_nextTrack = m_current + chunkSize;
		m_next = m_current;

		m_eventCount = 0;
		m_tickPosition = 0;
		return true;
	}
	return false;
}

bool MidiTraversal::checkNextChunk() const
{
	return strncmp((const char*)m_nextTrack, "MTrk", 4) == 0;
}

const unsigned char* MidiTraversal::findNextChunk() const
{
	return (const unsigned char*)strstr((const char*)m_current, "MTrk");
}

bool MidiTraversal::doesNextTrackExist()
{
	return m_nextTrack < m_end;
}

void MidiTraversal::setNextTrack(const unsigned char* location)
{
	if (location)
		m_nextTrack = location;
	else
		m_nextTrack = m_end;
}

bool MidiTraversal::next()
{
	m_current = m_next;
	if (m_current < m_nextTrack)
	{
		++m_eventCount;
		m_tickPosition += VariableLengthQuantity(m_current);
		if (*m_current == 0xFF || *m_current == 0xF7 || *m_current == 0xF0)
		{
			if (*m_current == 0xFF)
				++m_current;

			m_eventType = *m_current++;
			// Will and SHOULD move m_current before the addition is applied
			m_next = m_current + VariableLengthQuantity(m_current);
		}
		else
		{
			if (*m_current >= 128)
				m_midiEvent = *m_current++;
			else if (m_midiEvent == 0)
				throw "you dun goofed";

			m_eventType = m_midiEvent;
			switch (m_eventType)
			{
			case 0x80:
			case 0x90:
			case 0xB0:
			case 0xA0:
			case 0xE0:
			case 0xF2:
				m_next = m_current + 2;
				break;
			case 0xC0:
			case 0xD0:
			case 0xF3:
				m_next = m_current + 1;
				break;
			default:
				m_next = m_current;
			}
		}

		return true;
	}
	else if (m_current > m_nextTrack)
		m_current = m_nextTrack;
	return false;
}

void MidiTraversal::move(size_t count)
{
	if (m_current + count <= m_next)
		m_current += count;
	else
		m_current = m_next;
}

void MidiTraversal::skipTrack()
{
	m_current = m_nextTrack;
}

std::string MidiTraversal::extractText()
{
	std::string str((const char*)m_current, m_next - m_current);
	m_current = m_next;
	return str;
}

bool MidiTraversal::extract(unsigned char& value)
{
	if (m_current < m_next)
	{
		value = *m_current++;
		return true;
	}
	return false;
}

unsigned char MidiTraversal::extract()
{
	if (m_current == m_next)
		throw NoParseException();

	return *m_current++;
}
