#pragma once
#include "BasicTrack.h"
#include "Difficulty/Difficulty_bch.hpp"

template <class T>
bool BasicTrack<T>::save_bch(std::fstream& outFile) const
{
	if (!occupied())
		return false;

	outFile.write("INST", 4);
	uint32_t length = 0;
	auto start = outFile.tellp();
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	char numDiffs = 0;
	outFile.put(numDiffs);

	for (int diff = 4; diff >= 0; --diff)
		if (m_difficulties[diff].occupied())
		{
			m_difficulties[diff].save_bch(outFile);
			++numDiffs;
		}

	outFile.write("ANIM", 4);
	length = 4;
	outFile.write((char*)&length, 4);
	uint32_t count = 0;
	outFile.write((char*)&count, 4);

	auto end = outFile.tellp();
	length = uint32_t(end - start - 4);
	outFile.seekp(start);
	outFile.write((char*)&length, 4);
	outFile.put(m_instrumentID);
	outFile.put(numDiffs);
	outFile.seekp(end);
	return true;
}
