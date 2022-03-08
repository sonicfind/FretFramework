#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MetaEvent::MetaEvent(unsigned char type, std::fstream& inFile, bool read)
		: SysexEvent((char)0xFF, inFile, read)
		, m_type(type) {}

	uint32_t MidiChunk_Track::MetaEvent::getSize() const
	{
		return SysexEvent::getSize() + 1;
	}

	MidiChunk_Track::MetaEvent_Text::MetaEvent_Text(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile, true)
	{
		m_text = m_data;
	}

	MidiChunk_Track::MetaEvent_ChannelPrefix::MetaEvent_ChannelPrefix(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_prefix, 1);
	}

	MidiChunk_Track::MetaEvent_Tempo::MetaEvent_Tempo(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_microsecondsPerQuarter + 1, m_length);
		m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
	}

	MidiChunk_Track::MetaEvent_SMPTE::MetaEvent_SMPTE(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_smpte, m_length);
	}

	MidiChunk_Track::MetaEvent_TimeSignature::MetaEvent_TimeSignature(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_timeSig, m_length);
	}

	MidiChunk_Track::MetaEvent_KeySignature::MetaEvent_KeySignature(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_keySig, m_length);
	}

	MidiChunk_Track::MetaEvent_Data::MetaEvent_Data(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile, true)
	{
	}
}
