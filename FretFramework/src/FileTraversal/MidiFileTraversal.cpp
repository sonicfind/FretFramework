#include "MidiFileTraversal.h"

MidiTraversal::MidiTraversal(const FilePointers& file)
	: Traversal(file)
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

void MidiTraversal::skipTrack()
{
	m_current = m_nextTrack;
}
