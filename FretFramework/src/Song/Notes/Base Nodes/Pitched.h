#pragma once
#include "Sustainable.h"

class Pitched : public Sustainable
{
protected:
	// Valid pitches: 36 - 84 (83 for RB Blitz)
	char m_pitch = 0;

public:
	Toggleable& m_isPitched = m_isActive;
	Pitched() = default;
	Pitched(const Pitched& other);
	Pitched& operator=(const Pitched& other);
	void setPitch(char pitch);
	char getPitch() const { return m_pitch; }
	void save_pitch_cht(std::stringstream& buffer) const;
	bool save_pitch_cht(int lane, std::stringstream& buffer) const;
	void save_pitch_bch(char*& outPtr) const;
	bool save_pitch_bch(int lane, char*& outPtr) const;
};
