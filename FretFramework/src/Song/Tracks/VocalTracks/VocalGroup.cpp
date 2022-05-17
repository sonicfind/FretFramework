#include "VocalGroup.h"
#include "../VariableLengthQuantity.h"

void VocalGroup<1>::save_cht(uint32_t position, std::fstream& outFile)
{
	outFile << '\t' << position << " = N";
	m_vocals[0]->save_cht(1, outFile);
	m_vocals[0]->save_pitch_cht(outFile);
	outFile << '\n';
}

uint32_t VocalGroup<1>::save_bch(uint32_t position, std::fstream& outFile)
{
	// Event type - Single
	const char type = 9;
	static char buffer[262] = {};
	char* current = buffer;

	m_vocals[0]->save_bch(1, current);
	m_vocals[0]->save_pitch_bch(current);

	VariableLengthQuantity(position).writeToFile(outFile);
	outFile.put(type);
	VariableLengthQuantity length(uint32_t(current - buffer));
	length.writeToFile(outFile);
	outFile.write(buffer, length);
	return 1;
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

uint32_t VocalGroup<3>::save_bch(uint32_t position, std::fstream& outFile)
{
	VariableLengthQuantity quantity(position);
	char type;
	VariableLengthQuantity length(0);
	static char buffer[772] = { 0, 0, 0, 0 };
	char* current = buffer + 1;

	// Writes all the main note data to the buffer starting at index 7
	char numActive = 0;
	for (int lane = 0; lane < 3; ++lane)
		if (m_vocals[lane])
		{
			m_vocals[lane]->save_bch(lane + 1, current);
			++numActive;
		}

	int numEvents = 1;
	if (numActive == 1)
	{
		// Event type - Single (Lyric)
		type = 9;

		int lane = 0;
		while (!m_vocals[lane])
			++lane;
		m_vocals[lane]->save_pitch_bch(current);

		length = uint32_t(current - (buffer + 1));
		memcpy(buffer, buffer + 1, length);
	}
	else
	{
		// Event type - Chord (Lyric)
		type = 10;
		length = uint32_t(current - buffer);
		buffer[0] = numActive;
	}

	quantity.writeToFile(outFile);
	outFile.put(type);
	length.writeToFile(outFile);
	outFile.write(buffer, length);
	if (numActive > 1)
	{
		// Event type - Vocalize
		buffer[0] = 0;
		current = buffer + 1;

		for (int lane = 0; lane < 3; ++lane)
			if (m_vocals[lane] && m_vocals[lane]->save_pitch_bch(lane + 1, current))
				++buffer[0];

		if (buffer[0])
		{
			numEvents = 2;
			quantity = 0;
			type = 11;
			length = uint32_t(current - buffer);
			quantity.writeToFile(outFile);
			outFile.put(type);
			length.writeToFile(outFile);
			outFile.write(buffer, length);
		}
	}
	return numEvents;
}
