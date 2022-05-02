#include "Effect.h"

SustainableEffect::SustainableEffect(uint32_t duration)
	: m_duration(duration) {}

void StarPowerPhrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 2 " << m_duration << '\n';
}

char StarPowerPhrase::getMidiNote() const
{
	return 116;
}

void StarPowerActivation::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 64 " << m_duration << '\n';
}

char StarPowerActivation::getMidiNote() const
{
	// Will notify the midiTrackFiller to fill all of BRE's lanes
	return -1;
}

void Solo::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 3 " << m_duration << '\n';
}

char Solo::getMidiNote() const
{
	return 103;
}

void Tremolo::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 65 " << m_duration << '\n';
}

char Tremolo::getMidiNote() const
{
	return 126;
}

void Trill::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 66 " << m_duration << '\n';
}

char Trill::getMidiNote() const
{
	return 127;
}

void LyricLine::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 4 " << m_duration << '\n';
}

char LyricLine::getMidiNote() const
{
	return 105;
}

void RangeShift::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 5 " << m_duration << '\n';
}

char RangeShift::getMidiNote() const
{
	return 0;
}

void HarmonyPhrase::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 6 " << m_duration << '\n';
}

char HarmonyPhrase::getMidiNote() const
{
	return 106;
}

void LyricShift::save_cht(uint32_t position, std::fstream& outFile, const char* const tabs) const
{
	outFile << tabs << position << " = S 67 " << m_duration << '\n';
}

char LyricShift::getMidiNote() const
{
	return 1;
}
