#pragma once
#include "Sustainable.h"

class Pitched : public Sustainable
{
	using Hittable::m_isActive;
protected:
	// Valid pitches: 36 - 84 (83 for RB Blitz)
	char m_pitch = 0;

public:
	Toggleable& m_isSung = m_isActive;
	Pitched& operator=(const Pitched& other);
	void setPitch(char pitch);
	char getPitch() const { return m_pitch; }
	void save_pitch_cht(std::fstream& outFile) const;
	void save_pitch_cht(int lane, std::fstream& outFile) const;
	void save_pitch_bch(char*& outPtr) const;
	bool save_pitch_bch(int lane, char*& outPtr) const;
};
