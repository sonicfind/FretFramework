#include "Note_cht.hpp"
#include "DrumNote.h"

void DrumNote::init_chartV1(unsigned char lane, uint32_t sustain)
{
	if (lane == 0)
		m_special.init(sustain);
	else if (lane <= 5)
	{
		m_colors[lane - 1].init(sustain);
		if (lane == 5)
			s_is5Lane = true;
	}
	else if (lane == 32)
		m_special.m_isDoubleBass = true;
	else if (lane >= 66 && lane <= 68)
		m_colors[lane - 65].modify('C');
	else
		throw InvalidNoteException(lane);
}

void DrumNote::save_cht(const uint32_t position, std::fstream& outFile) const
{
	uint32_t numActive = Note<5, DrumPad_Pro, DrumPad_Bass>::write_notes(position, outFile);

	int numMods = m_isFlamed ? 1 : 0;
	if (m_special && m_special.m_isDoubleBass)
		++numMods;

	for (int i = 0; i < 5; ++i)
	{
		if (m_colors[i])
		{
			if (i < 4 && m_colors[i].m_isCymbal)
				++numMods;

			if (m_colors[i].m_isAccented || m_colors[i].m_isGhosted)
				++numMods;
		}
	}

	if (numMods > 0)
	{
		if (numActive == 1)
		{
			outFile << ' ' << numMods;

			if (m_special)
				m_special.save_modifier_cht(outFile);
			else
			{
				if (m_isFlamed)
					outFile << " F";

				if (m_colors[4])
					reinterpret_cast<const DrumPad*>(&m_colors[4])->save_modifier_cht(outFile);
				else
				{
					int i = 0;
					while (!m_colors[i])
						++i;
					m_colors[i].save_modifier_cht(outFile);
				}
			}
		}
		else
		{
			outFile << "\n\t\t" << position << " = M " << numMods;

			if (m_isFlamed)
				outFile << " F";

			if (m_special)
				m_special.save_modifier_cht(0, outFile);

			for (int i = 0; i < 5; ++i)
				if (m_colors[i])
				{
					if (i == 4)
						reinterpret_cast<const DrumPad*>(&m_colors[4])->save_modifier_cht(i + 1, outFile);
					else
						m_colors[i].save_modifier_cht(i + 1, outFile);
				}
		}
	}
	outFile << '\n';
}
