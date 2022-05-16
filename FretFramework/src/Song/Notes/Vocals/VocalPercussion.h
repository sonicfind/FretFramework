#pragma once
#include "Base Nodes/Modifiable.h"

// m_isActive will be used to determine whether the note is "playable"
class VocalPercussion : public Hittable
{
	using Hittable::m_isActive;

public:
	Toggleable& m_isPlayable = m_isActive;
	VocalPercussion();
	VocalPercussion(const VocalPercussion&) = default;
	bool modify(char modifier);
	void save_modifier_cht(int lane, std::fstream& outFile) const;
	void save_cht(std::fstream& outFile) const;
};
