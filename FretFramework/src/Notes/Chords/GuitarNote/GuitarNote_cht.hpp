#pragma once
#include "InstrumentalNote_cht.hpp"
#include "GuitarNote.h"

template<int numColors>
int GuitarNote<numColors>::write_modifiers_single(std::stringstream& buffer) const
{
	int numMods = 0;
	if (m_isForced != ForceStatus::UNFORCED)
	{
		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			buffer << " F";
			break;
		case ForceStatus::HOPO_ON:
			buffer << " <";
			break;
		case ForceStatus::HOPO_OFF:
			buffer << " >";
		}
		numMods = 1;
	}

	if (m_isTap)
	{
		++numMods;
		buffer << " T";
	}
	return numMods;
}

template<int numColors>
int GuitarNote<numColors>::write_modifiers_chord(std::stringstream& buffer) const
{
	int numMods = 0;
	if (m_isForced != ForceStatus::UNFORCED)
	{
		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			buffer << " F";
			break;
		case ForceStatus::HOPO_ON:
			buffer << " <";
			break;
		case ForceStatus::HOPO_OFF:
			buffer << " >";
		}
		numMods = 1;
	}

	if (m_isTap)
	{
		++numMods;
		buffer << " T";
	}
	return numMods;
}
