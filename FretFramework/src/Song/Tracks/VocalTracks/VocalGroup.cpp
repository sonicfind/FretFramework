#include "VocalGroup.h"

void VocalGroup<1>::save_cht(uint32_t position, std::fstream& outFile)
{
	outFile << '\t' << position << " = N";
	m_vocals[0]->save_cht(1, outFile);
	m_vocals[0]->save_pitch_cht(outFile);
	outFile << '\n';
}

void VocalGroup<3>::save_cht(uint32_t position, std::fstream& outFile)
{
	int numActive = 0;
	for (const auto& vocal : m_vocals)
		if (vocal)
			++numActive;

	if (numActive == 1)
	{
		int lane = 0;
		while (!m_vocals[lane])
			++lane;

		outFile << '\t' << position << " = N";
		m_vocals[lane]->save_cht(lane + 1, outFile);
		m_vocals[lane]->save_pitch_cht(outFile);
	}
	else
	{
		outFile << '\t' << position << " = C " << numActive;
		int numSung = 0;
		for (int lane = 0; lane < 3; ++lane)
			if (m_vocals[lane])
			{
				m_vocals[lane]->save_cht(lane + 1, outFile);
				if (m_vocals[lane]->m_isSung)
					++numSung;
			}

		if (numSung > 0)
		{
			outFile << "\n\t" << position << " = V " << numSung;
			for (int lane = 0; lane < 3; ++lane)
				if (m_vocals[lane] && m_vocals[lane]->m_isSung)
					m_vocals[lane]->save_pitch_cht(lane + 1, outFile);
		}
	}
	outFile << '\n';
}
