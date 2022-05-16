#pragma once
#include "Note_cht.hpp"
#include "Chord.h"

// write values to a V2 .chart file
template <int numColors>
inline void Chord<numColors>::save_cht(const uint32_t position, std::fstream& outFile) const
{
	int numActive = Note<numColors, Sustainable, Sustainable>::write_notes_cht(position, outFile);
	int numMods = m_isForced != ForceStatus::UNFORCED ? 1 : 0;
	if (m_isTap)
		++numMods;

	if (numMods > 0)
	{
		if (numActive > 1)
			outFile << "\n\t\t" << position << " = M";
		outFile << ' ' << numMods;

		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			outFile << " F";
			break;
		case ForceStatus::HOPO_ON:
			outFile << " <";
			break;
		case ForceStatus::HOPO_OFF:
			outFile << " >";
		}

		if (m_isTap)
			outFile << " T";
	}
	outFile << '\n';
}
