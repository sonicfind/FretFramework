#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MetaEvent::MetaEvent(unsigned char type, std::fstream& inFile)
		: MidiEvent(0xFF)
		, m_type(type)
		, m_length(inFile) {}

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

	MidiChunk_Track::MetaEvent_ChannelPrefix::MetaEvent_ChannelPrefix(std::fstream& inFile)
		: MetaEvent(0x20, inFile)
	{
		m_prefix = (unsigned char)inFile.get();
	}

	MidiChunk_Track::MetaEvent_End::MetaEvent_End(std::fstream& inFile)
		: MetaEvent(0x2F, inFile) {}

	MidiChunk_Track::MetaEvent_Tempo::MetaEvent_Tempo(std::fstream& inFile)
		: MetaEvent(0x51, inFile)
	{
		// The value is saved in the file as a 24bit unsigned int
		// To properly read it, we must read in the value at a one character offset then flip the bytes
		inFile.read((char*)&m_microsecondsPerQuarter + 1, m_length);
		m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
	}

	MidiChunk_Track::MetaEvent_SMPTE::MetaEvent_SMPTE(std::fstream& inFile)
		: MetaEvent(0x54, inFile)
	{
		m_hour = (unsigned char)inFile.get();
		m_minute = (unsigned char)inFile.get();
		m_second = (unsigned char)inFile.get();
		m_frame = (unsigned char)inFile.get();
		m_subframe = (unsigned char)inFile.get();
	}

	MidiChunk_Track::MetaEvent_TimeSignature::MetaEvent_TimeSignature(std::fstream& inFile)
		: MetaEvent(0x58, inFile)
	{
		m_numerator = (unsigned char)inFile.get();
		m_denominator = (unsigned char)inFile.get();
		m_metronome = (unsigned char)inFile.get();
		m_32ndsPerQuarter = (unsigned char)inFile.get();
	}

	MidiChunk_Track::MetaEvent_KeySignature::MetaEvent_KeySignature(std::fstream& inFile)
		: MetaEvent(0x59, inFile)
	{
		m_numFlatsOrSharps = (unsigned char)inFile.get();
		m_scaleType = (unsigned char)inFile.get();
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
