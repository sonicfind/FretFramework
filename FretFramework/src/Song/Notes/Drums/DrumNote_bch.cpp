#include "Note_bch.hpp"
#include "DrumNote.h"

uint32_t DrumNote::save_bch(const uint32_t position, std::fstream& outFile) const
{
	WebType quantity(position);
	char type;
	WebType length(0);
	static char buffer[26] = { 0, 0, 0, 0 };
	char* current = buffer + 1;

	// Writes all the main note data to the buffer starting at index 1
	char numActive = write_notes_bch(current);

	if (numActive == 1)
	{
		type = 6;
		if (m_special)
			m_special.save_modifier_bch(current);
		else
		{
			char* base = current;
			if (m_colors[4])
				reinterpret_cast<const DrumPad*>(&m_colors[4])->save_modifier_bch(current);
			else
			{
				int i = 0;
				while (!m_colors[i])
					++i;
				m_colors[i].save_modifier_bch(current);
			}

			if (m_isFlamed)
			{
				*base++ = 1;
				current = base;
			}
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
	if (numActive > 1)
	{
		current = buffer + 1;
		if (m_isFlamed)
		{
			*current++ = 1;
			buffer[0] = 1;
		}
		else
			buffer[0] = 0;
		current[0] = 0;

		if (m_special && m_special.save_modifier_bch(0, current))
		{
			++buffer[0];
			current[0] = 0;
		}

		// Dedicated mod byte for each color
		for (int i = 0; i < 5; ++i)
			if (m_colors[i])
			{
				if ((i < 4 && m_colors[i].save_modifier_bch(i + 1, current)) ||
					(i == 4 && reinterpret_cast<const DrumPad*>(&m_colors[4])->save_modifier_bch(5, current)))
				{
					++buffer[0];
					current[0] = 0;
				}
			}

		if (buffer[0])
		{
			numEvents = 2;
			quantity = 0;
			type = 8;
			length = uint32_t(current - buffer);
			quantity.writeToFile(outFile);
			outFile.put(type);
			length.writeToFile(outFile);
			outFile.write(buffer, length);
		}
	}
	return numEvents;
}
