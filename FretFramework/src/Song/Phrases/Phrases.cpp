#include "Phrases.h"

Phrase::Phrase(char midi, char cht)
	: m_midiNote(midi)
	, m_chtType(cht) {}

void Phrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << (int)m_chtType << '\n';
}

SustainablePhrase::SustainablePhrase(char midi, char cht, uint32_t duration)
	: Phrase(midi, cht)
	, m_duration(duration) {}

void SustainablePhrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << (int)m_chtType << ' ' << m_duration << '\n';
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
