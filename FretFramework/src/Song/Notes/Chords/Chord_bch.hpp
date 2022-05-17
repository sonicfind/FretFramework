#pragma once
#include "Note_bch.hpp"
#include "Chord.h"

template<int numColors>
inline uint32_t Chord<numColors>::save_bch(const uint32_t position, std::fstream& outFile) const
{
	WebType quantity(position);
	char type;
	WebType length(0);
	static char buffer[1 + 5 * numColors] = { 0, 0, 0, 0 };
	char* current = buffer + 1;

	// Writes all the main note data to the buffer starting at index 1
	char numActive = Note<numColors, Sustainable, Sustainable>::write_notes_bch(current);

	if (numActive == 1)
	{
		type = 6;
		current[0] = 0;
		if (m_isForced != ForceStatus::UNFORCED || m_isTap)
		{
			switch (m_isForced)
			{
			case ForceStatus::FORCED:
				current[0] = 1;
				break;
			case ForceStatus::HOPO_ON:
				current[0] = 2;
				break;
			case ForceStatus::HOPO_OFF:
				current[0] = 4;
			}

			if (m_isTap)
				current[0] |= 8;
			++current;
		}

		length = uint32_t(current - (buffer + 1));
		memcpy(buffer, buffer + 1, length);
	}
	else
	{
		type = 7;
		length = uint32_t(current - buffer);
		buffer[0] = numActive;
	}

	quantity.writeToFile(outFile);
	outFile.put(type);
	length.writeToFile(outFile);
	outFile.write(buffer, length);
	uint32_t numEvents = 1;
	if (numActive > 1 && (m_isForced != ForceStatus::UNFORCED || m_isTap))
	{
		quantity = 0;
		type = 8;
		length = 2;
		buffer[0] = 1;
		switch (m_isForced)
		{
		case ForceStatus::FORCED:
			buffer[1] = 1;
			break;
		case ForceStatus::HOPO_ON:
			buffer[1] = 2;
			break;
		case ForceStatus::HOPO_OFF:
			buffer[1] = 4;
			break;
		default:
			buffer[1] = 0;
		}

		if (m_isTap)
			buffer[1] |= 8;

		quantity.writeToFile(outFile);
		outFile.put(type);
		length.writeToFile(outFile);
		outFile.write(buffer, 2);
		numEvents = 2;
	}
	return numEvents;
}
