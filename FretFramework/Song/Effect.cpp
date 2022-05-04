#include "Effect.h"

Effect::Effect(char midi, int cht)
	: m_midiNote(midi)
	, m_chtType(cht) {}

void Effect::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << m_chtType << '\n';
}

SustainableEffect::SustainableEffect(char midi, int cht, uint32_t duration)
	: Effect(midi, cht)
	, m_duration(duration) {}

void SustainableEffect::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S " << m_chtType << ' ' << m_duration << '\n';
}

StarPowerPhrase::StarPowerPhrase(uint32_t duration)
	: SustainableEffect(116, 2, duration) {}

StarPowerActivation::StarPowerActivation(uint32_t duration)
	: SustainableEffect(-1, 64, duration) {}

Solo::Solo(uint32_t duration)
	: SustainableEffect(103, 3, duration) {}

Tremolo::Tremolo(uint32_t duration)
	: SustainableEffect(126, 65, duration) {}

Trill::Trill(uint32_t duration)
	: SustainableEffect(127, 66, duration) {}

LyricLine::LyricLine(uint32_t duration)
	: SustainableEffect(105, 4, duration) {}

RangeShift::RangeShift(uint32_t duration)
	: SustainableEffect(0, 5, duration) {}

HarmonyLine::HarmonyLine(uint32_t duration)
	: SustainableEffect(106, 6, duration) {}

LyricShift::LyricShift(uint32_t duration)
	: SustainableEffect(1, 67, duration) {}
