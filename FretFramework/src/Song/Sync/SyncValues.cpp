#include "SyncValues.h"
SyncValues::SyncValues(bool markBPM, bool markTimeSig)
	: m_markBPM(markBPM)
	, m_markTimeSig(markTimeSig) {}

SyncValues::SyncValues()
	: SyncValues(false) {}

SyncValues& SyncValues::operator=(const SyncValues& sync)
{
	m_markBPM = false;
	m_markTimeSig = false;
	m_bpm = sync.m_bpm;
	m_timeSigNumerator = sync.m_timeSigNumerator;
	m_timeSigDenomExponent = sync.m_timeSigDenomExponent;
	return *this;
}

void SyncValues::writeSync_chart(const uint32_t position, std::fstream& outFile) const
{
	if (m_anchorPoint)
		outFile << '\t' << position << " = A " << m_anchorPoint << '\n';

	if (m_markTimeSig)
	{
		outFile << '\t' << position << " = TS " << m_timeSigNumerator;
		if (m_timeSigDenomExponent != 2)
			outFile << ' ' << m_timeSigDenomExponent;
		outFile << '\n';
	}

	if (m_markBPM)
		outFile << '\t' << position << " = B " << (uint32_t)roundf(m_bpm * 1000) << '\n';
}

void SyncValues::setBPM(float bpm)
{
	m_bpm = bpm;
	m_markBPM = true;
}

void SyncValues::setTimeSig(uint32_t numerator, uint32_t denominator)
{
	m_timeSigNumerator = numerator;
	m_timeSigDenomExponent = denominator;
	m_markTimeSig = true;
}

void SyncValues::setAnchor(uint32_t anchor)
{
	m_anchorPoint = anchor;
}

float SyncValues::getBPM() const
{
	if (m_markBPM)
		return m_bpm;
	else
		return 0;
}

std::pair<uint32_t, uint32_t> SyncValues::getTimeSig() const
{
	if (m_markTimeSig)
		return std::pair<uint32_t, uint32_t>(m_timeSigNumerator, m_timeSigDenomExponent);
	else
		return std::pair<uint32_t, uint32_t>();
}

uint32_t SyncValues::getAnchor() const
{
	return m_anchorPoint;
}

void SyncValues::unmarkBPM() { m_markBPM = false; }
void SyncValues::unmarkTimeSig() { m_markTimeSig = false; }
