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

		m_eventCount = 0;
		m_tickPosition = 0;
		m_midiEvent = 0;
		m_eventType = 0;

		const unsigned char* ev = m_current;
		uint32_t delta = VariableLengthQuantity(ev);
		unsigned char tmp = *ev++;

		if (tmp == 0xFF)
		{
			m_eventType = *ev++;
			if (m_eventType == 3)
			{
				m_current = ev;
				const uint32_t length = VariableLengthQuantity(m_current);
				m_trackname.assign((const char*)m_current, length);
				m_current += length;

				m_tickPosition = delta;
				++m_eventCount;
			}
			else if (m_eventType != 0x2F)
				m_trackname.clear();
			else
				return false;
		}

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
	m_nextTrack = location;
	if (!m_nextTrack)
		m_nextTrack = m_end;
}

bool MidiTraversal::next()
{
	if (m_current >= m_nextTrack)
		return false;

	m_tickPosition += VariableLengthQuantity(m_current);
	unsigned char tmp = *m_current++;
	if (tmp < 0xF0)
	{
		if (tmp >= 128)
		{
			m_channel = tmp & 15;
			m_midiEvent = tmp & 240;

			if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
			{
				m_note.midiValue = *m_current++;
				m_note.velocity = *m_current++;
			}
			else if (m_midiEvent == 0xB0)
			{
				m_controlChange.controller = *m_current++;
				m_controlChange.value = *m_current++;
			}
			else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0)
				m_current += 2;
			else
				++m_current;
		}
		else if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
		{
			m_note.midiValue = tmp;
			m_note.velocity = *m_current++;
		}
		else if (m_midiEvent == 0xB0)
		{
			m_controlChange.controller = tmp;
			m_controlChange.value = *m_current++;
		}
		else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0 || m_midiEvent == 0xF2)
			++m_current;
		else if (m_midiEvent != 0xC0 && m_midiEvent != 0xD0 && m_midiEvent != 0xF3)
			return false;

		m_eventType = m_midiEvent;
	}
	else if (tmp == 0xFF)
	{
		m_eventType = *m_current++;
		const uint32_t length = VariableLengthQuantity(m_current);
		if (m_eventType < 16)
			m_text.assign(m_current, m_current + length);
		else if (m_eventType == 0x51)
		{
			memcpy((char*)&m_microsecondsPerQuarter + 1, m_current, 3);
			m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
		}
		else if (m_eventType == 0x58)
			memcpy(&m_timeSig, m_current, 4);
		else if (m_eventType == 0x2F)
			return false;

		m_current += length;
	}
	else
	{
		m_eventType = tmp;
		if (tmp == 0xF7 || tmp == 0xF0)
		{
			uint32_t length = VariableLengthQuantity(m_current);
			m_sysex.assign((const char*)m_current, length);
			m_current += length;
		}
		else if (tmp == 0xF2)
		{
			m_midiEvent = m_eventType;
			m_current += 2;
		}
		else if (tmp == 0xF3)
		{
			m_midiEvent = m_eventType;
			++m_current;
		}
	}

	++m_eventCount;
	return true;
}

bool MidiTraversal::scanNext()
{
	if (m_current >= m_nextTrack)
		return false;

	m_tickPosition += VariableLengthQuantity(m_current);
	unsigned char tmp = *m_current++;
	if (tmp < 0xF0)
	{
		if (tmp >= 128)
		{
			m_channel = tmp & 15;
			m_midiEvent = tmp & 240;

			if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
			{
				m_note.midiValue = *m_current++;
				m_note.velocity = *m_current++;
			}
			else if (m_midiEvent == 0xB0)
			{
				m_controlChange.controller = *m_current++;
				m_controlChange.value = *m_current++;
			}
			else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0)
				m_current += 2;
			else
				++m_current;
		}
		else if (m_midiEvent == 0x80 || m_midiEvent == 0x90)
		{
			m_note.midiValue = tmp;
			m_note.velocity = *m_current++;
		}
		else if (m_midiEvent == 0xB0)
		{
			m_controlChange.controller = tmp;
			m_controlChange.value = *m_current++;
		}
		else if (m_midiEvent == 0xA0 || m_midiEvent == 0xE0 || m_midiEvent == 0xF2)
			++m_current;
		else if (m_midiEvent != 0xC0 && m_midiEvent != 0xD0 && m_midiEvent != 0xF3)
			return false;

		m_eventType = m_midiEvent;
	}
	else if (tmp == 0xFF)
	{
		m_eventType = *m_current++;
		if (m_eventType != 0x2F)
		{
			const uint32_t length = VariableLengthQuantity(m_current);
			m_current += length;
		}
		else
			return false;
	}
	else
	{
		m_eventType = tmp;
		if (tmp == 0xF7 || tmp == 0xF0)
		{
			uint32_t length = VariableLengthQuantity(m_current);
			m_sysex.assign((const char*)m_current, length);
			m_current += length;
		}
		else if (tmp == 0xF2)
		{
			m_midiEvent = m_eventType;
			m_current += 2;
		}
		else if (tmp == 0xF3)
		{
			m_midiEvent = m_eventType;
			++m_current;
		}
	}

	++m_eventCount;
	return true;
}

void MidiTraversal::skipTrack()
{
	m_current = m_nextTrack;
}
