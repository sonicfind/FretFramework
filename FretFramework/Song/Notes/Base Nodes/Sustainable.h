#pragma once
#include "Hittable.h"

class Sustainable : public Hittable
{
	static uint16_t s_forceThreshold;
	static uint16_t s_sustainThreshold;
protected:
	// Must take sustain gap into account
	uint32_t m_sustain = 0;
public:
	void init(uint32_t sustain = 0)
	{
		m_isActive = true;
		m_sustain = sustain;
	}
	uint32_t getSustain() const { return m_sustain; }
	void setSustain(uint32_t sustain) { m_sustain = sustain; }
	void save_cht(int lane, std::fstream& outFile) const;

	static void setForceThreshold(uint16_t threshold)
	{
		s_forceThreshold = threshold;
	}

	static void setsustainThreshold(uint16_t threshold)
	{
		s_sustainThreshold = threshold;
	}

	static float getForceThreshold()
	{
		return s_forceThreshold;
	}

	static float getsustainThreshold()
	{
		return s_sustainThreshold;
	}
};