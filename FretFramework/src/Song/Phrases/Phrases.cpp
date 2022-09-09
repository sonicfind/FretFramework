#include "Phrases.h"
#include "Variable Types/WebType.h"

Phrase::Phrase(char midi, char cht)
	: m_midiNote(midi)
	, m_chtType(cht) {}

void Phrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << (int)m_chtType << '\n';
}

void Phrase::save_bch(std::fstream& outFile) const
{
	// buffer[3] is always 0
	static char buffer[4] = { 5, 2 };
	buffer[2] = m_chtType;
	outFile.write(buffer, 4);
}

SustainablePhrase::SustainablePhrase(char midi, char cht, uint32_t duration)
	: Phrase(midi, cht)
	, m_duration(duration) {}

void SustainablePhrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << (int)m_chtType << ' ' << m_duration << '\n';
}

void SustainablePhrase::save_bch(std::fstream& outFile) const
{
	static char buffer[7] = { 5, 0, 0, 0, 0, 0, 0 };
	static char* start = buffer + 3;

	char* current = start;
	buffer[2] = m_chtType;
	WebType::copyToBuffer(m_duration, current);
	buffer[1] = (char)(current - start + 1);
	outFile.write(buffer, 2ULL + buffer[1]);
}

StarPowerPhrase::StarPowerPhrase(uint32_t duration)
	: SustainablePhrase(116, 2, duration) {}

StarPowerActivation::StarPowerActivation(uint32_t duration)
	: SustainablePhrase(-1, 64, duration) {}

Solo::Solo(uint32_t duration)
	: SustainablePhrase(103, 3, duration) {}

Tremolo::Tremolo(uint32_t duration)
	: SustainablePhrase(126, 65, duration) {}

Trill::Trill(uint32_t duration)
	: SustainablePhrase(127, 66, duration) {}

LyricLine::LyricLine(uint32_t duration)
	: SustainablePhrase(105, 4, duration) {}

RangeShift::RangeShift(uint32_t duration)
	: SustainablePhrase(0, 5, duration) {}

HarmonyLine::HarmonyLine(uint32_t duration)
	: SustainablePhrase(106, 6, duration) {}

LyricShift::LyricShift()
	: Phrase(1, 67) {}

Player1_FaceOff::Player1_FaceOff(uint32_t duration)
	: SustainablePhrase(105, 0, duration) {}

Player2_FaceOff::Player2_FaceOff(uint32_t duration)
	: SustainablePhrase(106, 1, duration) {}
