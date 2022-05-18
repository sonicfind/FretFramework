#pragma once
#include "Base Nodes/Hittable.h"

// m_isActive will be used to determine whether the note is "playable"
class VocalPercussion : public Hittable
{
public:
	Toggleable& m_isPlayable = m_isActive;
	VocalPercussion();
	VocalPercussion(const VocalPercussion& other);
	bool modify(char modifier);
	void save_cht(std::fstream& outFile) const;
	void save_bch(std::fstream& outFile) const;
};
