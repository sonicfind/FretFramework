#pragma once
#include <stdint.h>
#include <sstream>
#include <fstream>
class SyncValues
{
	float m_bpm = 120;
	uint32_t m_timeSigNumerator = 4;
	uint32_t m_timeSigDenomExponent = 2;

public:
	bool readSync_chart(std::stringstream& ss, SyncValues& prev);
	void writeSync_chart(const uint32_t position, std::fstream& outFile, const SyncValues* prev) const;
};