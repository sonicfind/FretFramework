#pragma once
#include "Base Nodes/Hittable.h"
#include <fstream>

// m_isActive will be used to determine whether the note is "playable"
class VocalPercussion : public Hittable
{
	static const char s_playableBuffer[3];
	static const char s_noiseBuffer[4];

public:
	Toggleable& m_isPlayable = m_isActive;
	VocalPercussion();
	VocalPercussion(const VocalPercussion& other);
	void modify(char modifier);
	void modify_binary(char modifier);
	const char* save_cht() const;
	void save_bch(std::fstream& outFile) const;
};
