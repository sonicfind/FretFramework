#include "SyncValues.h"
bool SyncValues::readSync(std::stringstream& ss, SyncValues& prev)
{
	std::string type;
	ss >> type;
	if (type[0] == 'T')
	{
		ss >> m_timeSigNumerator;
		ss >> m_timeSigDenomExponent;
		if (m_bpm == 120)
			m_bpm = prev.m_bpm;
	}
	else if (type[0] == 'B')
	{
		ss >> m_bpm;
		m_bpm *= .001f;
		if (m_timeSigNumerator == 4 && m_timeSigDenomExponent == 2)
		{
			m_timeSigNumerator = prev.m_timeSigNumerator;
			m_timeSigDenomExponent = prev.m_timeSigDenomExponent;
		}
	}
	else
		return false;
	prev = *this;
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
