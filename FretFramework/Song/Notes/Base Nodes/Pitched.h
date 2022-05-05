#pragma once
#include "Sustainable.h"

class Pitched : public Sustainable
{
protected:
	// Valid pitches: 36 - 84 (83 for RB Blitz)
	char m_pitch = 0;

public:
	void setPitch(char pitch);
	char getPitch() const { return m_pitch; }
	void save_pitch_cht(std::fstream& outFile) const;
	void save_pitch_cht(int lane, std::fstream& outFile) const;
};
