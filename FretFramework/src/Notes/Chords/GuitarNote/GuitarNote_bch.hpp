#pragma once
#include "InstrumentalNote_bch.hpp"
#include "GuitarNote.h"

template<int numColors>
void GuitarNote<numColors>::write_modifiers_single(char*& buffer) const
{
	buffer[0] = 0;
	if (m_isForced != ForceStatus::UNFORCED)
	{
		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			buffer[0] = 1;
			break;
		case ForceStatus::HOPO_ON:
			buffer[0] = 2;
			break;
		case ForceStatus::HOPO_OFF:
			buffer[0] = 4;
		}
	}

	if (m_isTap)
		buffer[0] |= 8;

	if (buffer[0] > 0)
		++buffer;
}

template<int numColors>
char GuitarNote<numColors>::write_modifiers_chord(char*& buffer) const
{
	buffer[0] = 0;
	if (m_isForced != ForceStatus::UNFORCED)
	{
		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			buffer[0] = 1;
			break;
		case ForceStatus::HOPO_ON:
			buffer[0] = 2;
			break;
		case ForceStatus::HOPO_OFF:
			buffer[0] = 4;
		}
	}

	if (m_isTap)
		buffer[0] |= 8;

	if (buffer[0] > 0)
	{
		++buffer;
		return 1;
	}
	return 0;
}
