#include "MidiFile.h"

namespace MidiFile
{
	MidiChunk_Track::MetaEvent::MetaEvent(unsigned char type, std::fstream& inFile)
		: MidiEvent(0xFF)
		, m_type(type)
		, m_length(inFile) {}

	MidiChunk_Track::MetaEvent::MetaEvent(unsigned char type, uint32_t length)
		: MidiEvent(0xFF)
		, m_type(type)
		, m_length(length) {}

	void MidiChunk_Track::MetaEvent::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		// Reordered overload that skips over the syntax check
		MidiEvent::writeToFile(outFile, prevSyntax);
		outFile.put(m_type);
		m_length.writeToFile(outFile);
	}

	MidiChunk_Track::MetaEvent_Text::MetaEvent_Text(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
	{
		char* tmp = new char[m_length + 1]();
		inFile.read(tmp, m_length);
		m_text = tmp;
		delete[m_length + 1] tmp;
	}

	MidiChunk_Track::MetaEvent_Text::MetaEvent_Text(unsigned char type, const std::string& text)
		: MetaEvent(type, (uint32_t)text.length())
	{
		m_text = text;
	}

	void MidiChunk_Track::MetaEvent_Text::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)m_text.data(), m_length);
	}

	MidiChunk_Track::MetaEvent_ChannelPrefix::MetaEvent_ChannelPrefix(std::fstream& inFile)
		: MetaEvent(0x20, inFile)
	{
		m_prefix = (unsigned char)inFile.get();
	}

	void MidiChunk_Track::MetaEvent_ChannelPrefix::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_prefix, 1);
	}

	MidiChunk_Track::MetaEvent_End::MetaEvent_End(std::fstream& inFile)
		: MetaEvent(0x2F, inFile) {}

	MidiChunk_Track::MetaEvent_End::MetaEvent_End()
		: MetaEvent(0x2F, 0) {}

	MidiChunk_Track::MetaEvent_Tempo::MetaEvent_Tempo(std::fstream& inFile)
		: MetaEvent(0x51, inFile)
	{
		// The value is saved in the file as a 24bit unsigned int
		// To properly read it, we must read in the value at a one character offset then flip the bytes
		inFile.read((char*)&m_microsecondsPerQuarter + 1, m_length);
		m_microsecondsPerQuarter = _byteswap_ulong(m_microsecondsPerQuarter);
	}

	MidiChunk_Track::MetaEvent_Tempo::MetaEvent_Tempo(uint32_t mpq)
		: MetaEvent(0x51, 3)
	{
		m_microsecondsPerQuarter = mpq;
	}

	void MidiChunk_Track::MetaEvent_Tempo::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		// The value must be saved in the file as a 24bit unsigned int
		// To properly do so, we must flip the bytes then write the value at a one character offset
		uint32_t value = _byteswap_ulong(m_microsecondsPerQuarter);
		outFile.write((char*)&value + 1, m_length);
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

	void MidiChunk_Track::MetaEvent_SMPTE::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_hour, m_length);
	}

	MidiChunk_Track::MetaEvent_TimeSignature::MetaEvent_TimeSignature(std::fstream& inFile)
		: MetaEvent(0x58, inFile)
	{
		m_numerator = (unsigned char)inFile.get();
		m_denominator = (unsigned char)inFile.get();
		m_metronome = (unsigned char)inFile.get();
		m_32ndsPerQuarter = (unsigned char)inFile.get();
	}

	MidiChunk_Track::MetaEvent_TimeSignature::MetaEvent_TimeSignature(unsigned char num, unsigned char denom, unsigned char met)
		: MetaEvent(0x58, 4)
		, m_numerator(num)
		, m_denominator(denom)
		, m_metronome(met)
		, m_32ndsPerQuarter(8) {}

	void MidiChunk_Track::MetaEvent_TimeSignature::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_numerator, m_length);
	}

	MidiChunk_Track::MetaEvent_KeySignature::MetaEvent_KeySignature(std::fstream& inFile)
		: MetaEvent(0x59, inFile)
	{
		m_numFlatsOrSharps = (unsigned char)inFile.get();
		m_scaleType = (unsigned char)inFile.get();
	}

	void MidiChunk_Track::MetaEvent_KeySignature::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_numFlatsOrSharps, m_length);
	}

	MidiChunk_Track::MetaEvent_Data::MetaEvent_Data(unsigned char type, std::fstream& inFile)
		: MetaEvent(type, inFile)
		, m_data(new char[m_length + 1]())
	{
		inFile.read(m_data, m_length);
	}

	void MidiChunk_Track::MetaEvent_Data::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write(m_data, m_length);
	}

	MidiChunk_Track::MetaEvent_Data::~MetaEvent_Data()
	{
		delete[m_length + 1] m_data;
	}
}
