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
	bool readSync(std::stringstream& ss);
	void writeSync(const uint32_t position, std::ofstream& outFile, const SyncValues* prev) const;
};