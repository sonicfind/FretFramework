#include "MidiFile.h"

namespace MidiFile
{
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

	MidiChunk_Track::MetaEvent_End::MetaEvent_End()
		: MetaEvent(0x2F, 0) {}

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

	void MidiChunk_Track::MetaEvent_KeySignature::writeToFile(unsigned char& prevSyntax, std::fstream& outFile) const
	{
		MetaEvent::writeToFile(prevSyntax, outFile);
		outFile.write((char*)&m_numFlatsOrSharps, m_length);
	}
}
