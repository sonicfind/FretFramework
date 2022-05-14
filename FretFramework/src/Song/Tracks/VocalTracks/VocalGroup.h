#pragma once
#include "Notes/Vocals/Vocal.h"

template <int numTracks>
struct VocalGroup
{
	const Vocal* m_vocals[numTracks] = { nullptr };
	void save_cht(uint32_t position, std::fstream& outFile);
};
