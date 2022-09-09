#pragma once
#include <stdint.h>
#include <fstream>
class SyncValues
{
	float m_bpm = 120;
	uint32_t m_timeSigNumerator = 4;
	uint32_t m_timeSigDenomExponent = 2;
	// Microseconds
	uint32_t m_anchorPoint = 0;

	bool m_markBPM;
	bool m_markTimeSig;

public:
	SyncValues(bool markBPM, bool markTimeSig = false);
	SyncValues();
	SyncValues copy() const;
	void writeSync_cht(const uint32_t position, std::fstream& outFile) const;
	uint32_t writeSync_bch(uint32_t delta, std::fstream& outFile) const;
	void setBPM(float bpm);
	void setTimeSig(uint32_t numerator, uint32_t denominator);
	void setAnchor(uint32_t anchor);
	float getBPM() const;
	std::pair<uint32_t, uint32_t> getTimeSig() const;
	uint32_t getAnchor() const;
	void unmarkBPM();
	void unmarkTimeSig();
};
