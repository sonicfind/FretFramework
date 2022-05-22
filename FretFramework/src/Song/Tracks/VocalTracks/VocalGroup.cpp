#include "VocalGroup.h"
#include "Variable Types/VariableLengthQuantity.h"
#include "Variable Types/WebType.h"

void VocalGroup<1>::save_cht(uint32_t position, std::fstream& outFile)
{
	std::stringstream buffer;
	m_vocals[0]->save_cht(1, buffer);
	m_vocals[0]->save_pitch_cht(buffer);
	outFile << '\t' << position << " = N" << buffer.rdbuf() << '\n';
}

uint32_t VocalGroup<1>::save_bch(std::fstream& outFile)
{
	static char buffer[263];
	char* current = buffer;

	m_vocals[0]->save_bch(1, current);
	m_vocals[0]->save_pitch_bch(current);

	const uint32_t length(uint32_t(current - buffer));
	// Event type - Single (Lyric)
	outFile.put(9);
	WebType::writeToFile(length, outFile);
	outFile.write(buffer, length);
	return 1;
}

void VocalGroup<3>::save_cht(uint32_t position, std::fstream& outFile)
{
	std::stringstream buffer;
	int numActive = 0;
	for (int lane = 0; lane < 3; ++lane)
		if (m_vocals[lane])
		{
			m_vocals[lane]->save_cht(lane + 1, buffer);
			++numActive;
		}

	if (numActive == 1)
	{
		int lane = 0;
		while (!m_vocals[lane])
			++lane;
		m_vocals[lane]->save_pitch_cht(buffer);
		outFile << '\t' << position << " = N" << buffer.rdbuf() << '\n';
	}
	else
	{
		outFile << '\t' << position << " = C " << numActive << buffer.rdbuf() << '\n';
		buffer.clear();

		int numSung = 0;
		for (int lane = 0; lane < 3; ++lane)
			if (m_vocals[lane] && m_vocals[lane]->save_pitch_cht(lane + 1, buffer))
				++numSung;

		if (numSung > 0)
			outFile << "\t" << position << " = V " << numSung << buffer.rdbuf() << '\n';
	}
}

uint32_t VocalGroup<3>::save_bch(std::fstream& outFile)
{
	static char buffer[771];
	static char* const start = buffer + 1;
	char* current = start;

	// Writes all the main vocal data to the buffer starting at index 1
	buffer[0] = 0;
	for (int lane = 0; lane < 3; ++lane)
		if (m_vocals[lane])
		{
			m_vocals[lane]->save_bch(lane + 1, current);
			++buffer[0];
		}

	int numEvents = 1;
	if (buffer[0] == 1)
	{
		int lane = 0;
		while (!m_vocals[lane])
			++lane;
		m_vocals[lane]->save_pitch_bch(current);
		const uint32_t length(uint32_t(current - start));

		// Event type - Single (Lyric)
		outFile.put(9);
		WebType::writeToFile(length, outFile);
		outFile.write(start, length);
	}
	else
	{
		const uint32_t length(uint32_t(current - buffer));

		// Event type - Chord (Lyric)
		outFile.put(10);
		WebType::writeToFile(length, outFile);
		outFile.write(buffer, length);

		buffer[3] = 0;
		current = buffer + 4;

		for (int lane = 0; lane < 3; ++lane)
			if (m_vocals[lane] && m_vocals[lane]->save_pitch_bch(lane + 1, current))
				++buffer[3];

		if (buffer[3] > 0)
		{
			buffer[0] = 0;
			// Event type - Vocalize
			buffer[1] = 11;
			buffer[2] = char(current - buffer - 3);
			outFile.write(buffer, buffer[2] + 3ULL);
		}
	}
	return numEvents;
}
