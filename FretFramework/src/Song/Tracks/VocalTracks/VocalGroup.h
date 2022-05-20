#pragma once
#include "Song/Notes/Vocals/Vocal.h"
#include <fstream>

template <int numTracks>
struct VocalGroup
{
	const Vocal* m_vocals[numTracks] = { nullptr };
	void save_cht(uint32_t position, std::fstream& outFile);
	uint32_t save_bch(uint32_t position, std::fstream& outFile);
};
