#include "SyncValues.h"
bool SyncValues::readSync(std::stringstream& ss)
{
	std::string type;
	ss >> type;
	if (type[0] == 'T')
	{
		ss >> m_timeSigNumerator;
		ss >> m_timeSigDenomExponent;
	}
	else if (type[0] == 'B')
	{
		ss >> m_bpm;
		m_bpm *= .001f;
	}
	else
		return false;
	return true;
}

void SyncValues::writeSync(const uint32_t position, std::ofstream& outFile, const SyncValues* prev) const
{
	if (!prev || prev->m_timeSigNumerator != m_timeSigNumerator || prev->m_timeSigDenomExponent != m_timeSigDenomExponent)
	{
		outFile << "  " << position << " = TS " << m_timeSigNumerator;
		if (m_timeSigDenomExponent != 2)
			outFile << m_timeSigDenomExponent;
		outFile << '\n';
	}

	if (!prev || prev->m_bpm != m_bpm)
		outFile << "  " << position << " = B " << m_bpm * 1000 << '\n';
}
