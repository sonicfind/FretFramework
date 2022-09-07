#include "SyncValues.h"
SyncValues::SyncValues(bool markBPM, bool markTimeSig)
	: m_markBPM(markBPM)
	, m_markTimeSig(markTimeSig) {}

SyncValues::SyncValues()
	: SyncValues(false) {}

SyncValues SyncValues::copy() const
{
	SyncValues sync(*this);
	sync.m_bpm = false;
	sync.m_markTimeSig = false;
	sync.m_anchorPoint = 0;
	return sync;
}

void SyncValues::writeSync_cht(const uint32_t position, std::fstream& outFile) const
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

#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

uint32_t SyncValues::writeSync_bch(const uint32_t position, std::fstream& outFile) const
{
	static const WebType lengths[2] = { 2, 4 };
	WebType quantity(position);
	uint32_t numEvents = 0;
	if (m_markTimeSig)
	{
		quantity.writeToFile(outFile);
		outFile.put(2);
		lengths[0].writeToFile(outFile);
		outFile.put((unsigned char)m_timeSigNumerator);
		outFile.put((unsigned char)m_timeSigDenomExponent);
		numEvents = 1;
		quantity = 0;
	}

	if (m_markBPM)
	{
		quantity.writeToFile(outFile);
		outFile.put(1);
		lengths[1].writeToFile(outFile);
		uint32_t microsecondsPerQuarter = (uint32_t)roundf(60000000.0f / m_bpm);
		outFile.write((char*)&microsecondsPerQuarter, 4);
		++numEvents;
	}
	return numEvents;
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
