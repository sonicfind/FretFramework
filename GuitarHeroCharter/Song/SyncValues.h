#pragma once
#include <stdint.h>
#include <sstream>
#include <fstream>
class SyncValues
{
	float m_bpm = 120;
	uint32_t m_timeSigNumerator = 4;
	uint32_t m_timeSigDenomExponent = 2;

	bool m_markBPM;
	bool m_markTimeSig;

public:
	SyncValues(bool markBPM, bool markTimeSig = false);
	SyncValues();
	SyncValues(const SyncValues& sync) = default;
	SyncValues& operator=(const SyncValues& sync);
	void writeSync_chart(const uint32_t position, std::fstream& outFile) const;
	void setBPM(float bpm);
	void setTimeSig(uint32_t numerator, uint32_t denominator);
	float getBPM() const;
	std::pair<uint32_t, uint32_t> getTimeSig() const;
	void unmarkBPM();
	void unmarkTimeSig();
};
