#include "Effect.h"
Effect::Effect(uint32_t duration)
	: m_duration(duration) {}

void StarPowerPhrase::save_chart(uint32_t position, std::fstream& outFile)
{
	outFile << "  " << position << " = S " << m_duration << '\n';
}

void StarPowerActivation::save_chart(uint32_t position, std::fstream& outFile)
{
	outFile << "  " << position << " = A " << m_duration << '\n';
}

void Tremolo::save_chart(uint32_t position, std::fstream& outFile)
{
	outFile << "  " << position << " = R " << m_duration << '\n';
}

void Trill::save_chart(uint32_t position, std::fstream& outFile)
{
	outFile << "  " << position << " = D " << m_duration << '\n';
}
