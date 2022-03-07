#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MetaEvent::MetaEvent(char type, std::fstream& inFile)
		: SysexEvent(0xFF, inFile)
		, m_type(type) {}

	uint32_t MidiChunk_Track::MetaEvent::getSize() const
	{
		return SysexEvent::getSize() + 1;
	}

	void MidiChunk_Track::MetaEvent_Text::fillFromFile(std::fstream& inFile)
	{
		SysexEvent::fillFromFile(inFile);
		m_text = m_data;
	}

	void MidiChunk_Track::MetaEvent_ChannelPrefix::fillFromFile(std::fstream& inFile)
	{
		inFile >> m_prefix;
	}

	void MidiChunk_Track::MetaEvent_End::fillFromFile(std::fstream& inFile)
	{
		inFile.ignore(1);
	}

	void MidiChunk_Track::MetaEvent_Tempo::fillFromFile(std::fstream& inFile)
	{
		inFile.read((char*)&m_microsecondsPerQuarter + 1, m_length);
		m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
	}

	void MidiChunk_Track::MetaEvent_SMPTE::fillFromFile(std::fstream& inFile)
	{
		inFile.read((char*)&m_smpte, m_length);
	}

	void MidiChunk_Track::MetaEvent_TimeSignature::fillFromFile(std::fstream& inFile)
	{
		inFile.read((char*)&m_timeSig, m_length);
	}

	void MidiChunk_Track::MetaEvent_KeySignature::fillFromFile(std::fstream& inFile)
	{
		inFile.read((char*)&m_keySig, m_length);
	}
}
