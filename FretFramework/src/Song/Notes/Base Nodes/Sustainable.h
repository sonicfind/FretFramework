#pragma once
#include "Hittable.h"
#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

class Sustainable : public Hittable
{
	static uint16_t s_forceThreshold;
	static uint16_t s_sustainThreshold;
protected:
	// Must take sustain gap into account
	WebType m_sustain;

public:
	constexpr explicit Sustainable() : Hittable() {}
	void init(uint32_t sustain)
	{
		m_isActive = true;
		m_sustain = sustain;
	}
	uint32_t getSustain() const { return m_sustain; }
	void setSustain(uint32_t sustain) { m_sustain = sustain; }
	void save_cht(int lane, std::stringstream& buffer) const;
	void save_bch(int lane, char*& outPtr) const;
	void operator*=(float multiplier) { m_sustain = uint32_t(m_sustain * multiplier); }

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
