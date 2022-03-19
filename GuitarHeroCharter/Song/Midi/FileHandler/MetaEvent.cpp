#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MetaEvent::MetaEvent(unsigned char type, std::fstream& inFile)
		: MidiEvent(0xFF)
		, m_type(type)
		, m_length(inFile) {}

	uint32_t MidiChunk_Track::MetaEvent::getSize() const
	{
		return m_length.getValue() + m_length.getSize() + 2;
	}

	MidiChunk_Track::MetaEvent_Text::MetaEvent_Text(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		char* tmp = new char[m_length + 1]();
		inFile.read(tmp, m_length);
		m_text = tmp;
	}

	MidiChunk_Track::MetaEvent_Text::~MetaEvent_Text()
	{
		delete[m_length + 1] m_text.data();
	}

	MidiChunk_Track::MetaEvent_ChannelPrefix::MetaEvent_ChannelPrefix(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		inFile.read((char*)&m_prefix, 1);
	}

	MidiChunk_Track::MetaEvent_Tempo::MetaEvent_Tempo(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		// The value is saved in the file as a 24bit unsigned int
		// To properly read it, we must read in the value at a one character offset then flip the bytes
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
		: MetaEvent(type, inFile)
		, m_data(new char[m_length + 1]())
	{
		inFile.read(m_data, m_length);
	}

	MidiChunk_Track::MetaEvent_Data::~MetaEvent_Data()
	{
		delete[m_length + 1] m_data;
	}
}
