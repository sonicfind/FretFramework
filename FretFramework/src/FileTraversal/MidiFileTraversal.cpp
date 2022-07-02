#include "MidiFileTraversal.h"
#include "Variable Types/VariableLengthQuantity.h"

MidiTraversal::MidiTraversal(const std::filesystem::path& path)
	: Traversal(path)
	, m_nextTrack(m_current)
{
	if (strncmp((const char*)m_current, "MThd", 4) != 0)
		throw InvalidChunkTagException("MThd");

	m_current += 4;
	uint32_t chunkSize = _byteswap_ulong(*reinterpret_cast<const uint32_t*>(m_current));

	if (chunkSize != 6)
		throw "Invalid size of header chunk";

	m_current += 4;
	m_nextTrack = m_current + chunkSize;

	m_format = _byteswap_ushort(* reinterpret_cast<const uint16_t*>(m_current));
	m_current += 2;
	m_numTracks = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(m_current)) + 1;
	m_current += 2;
	m_tickRate = _byteswap_ushort(*reinterpret_cast<const uint16_t*>(m_current));
	m_current += 2;
}

bool MidiTraversal::validateChunk()
{
	if (strncmp((const char*)m_current, "MTrk", 4) == 0)
	{
		++m_trackCount;
		m_current += 4;
		uint32_t chunkSize = _byteswap_ulong(*reinterpret_cast<const uint32_t*>(m_current));
		m_current += 4;
		m_nextTrack = m_current + chunkSize;
		m_next = m_current;

		m_eventCount = 0;
		m_tickPosition = 0;
		m_midiEvent = 0;
		m_eventType = 0;
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
	if (m_next >= m_nextTrack)
		return false;

	m_current = m_next;
	m_tickPosition += VariableLengthQuantity(m_current);
	m_eventType = *m_current;
	switch (m_eventType)
	{
	case 0xFF:
		m_eventType = *++m_current;
		__fallthrough;
	case 0xF7:
	case 0xF0:
	{
		++m_current;
		uint32_t length = VariableLengthQuantity(m_current);
		m_next = m_current + length;
		break;
	}
	default:
		if (m_eventType >= 128)
		{
			m_midiEvent = m_eventType;
			++m_current;
			if (m_midiEvent < 240)
			{
				m_channel = m_midiEvent & 15;
				m_midiEvent -= m_channel;
			}
		}

		if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
		{
			m_midiNote = m_current[0];
			m_velocity = m_current[1];
			m_next = m_current + 2;
		}
		else if (m_midiEvent > 0)
		{
			switch (m_midiEvent)
			{
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
			}
		}
		else
			return false;

		m_eventType = m_midiEvent;
	}

	++m_eventCount;
	return true;
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
	m_next = m_current;
}

std::string MidiTraversal::extractText()
{
	if (m_current > m_next)
		throw NoParseException();

	std::string str((const char*)m_current, m_next - m_current);
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

unsigned char MidiTraversal::extractChar()
{
	if (m_current == m_next)
		throw NoParseException();

	return *m_current++;
}
