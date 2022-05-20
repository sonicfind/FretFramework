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

uint32_t VocalGroup<1>::save_bch(uint32_t position, std::fstream& outFile)
{
	// Event type - Single
	const char type = 9;
	static char buffer[262] = {};
	char* current = buffer;

	m_vocals[0]->save_bch(1, current);
	m_vocals[0]->save_pitch_bch(current);

	WebType(position).writeToFile(outFile);
	outFile.put(type);
	WebType length(uint32_t(current - buffer));
	length.writeToFile(outFile);
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
			outFile << "\n\t" << position << " = V " << numSung << buffer.rdbuf() << '\n';
	}
}

uint32_t VocalGroup<3>::save_bch(uint32_t position, std::fstream& outFile)
{
	static char buffer[771] = { 0, 0, 0, 0 };
	static char* const start = buffer + 1;
	char* current = start;

	// Writes all the main note data to the buffer starting at index 7
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

		WebType(position).writeToFile(outFile);
		// Event type - Single (Lyric)
		outFile.put(9);
		WebType length(uint32_t(current - start));
		length.writeToFile(outFile);
		outFile.write(start, length);
	}
	else
	{
		WebType(position).writeToFile(outFile);
		// Event type - Chord (Lyric)
		outFile.put(10);
		WebType length(uint32_t(current - buffer));
		length.writeToFile(outFile);
		outFile.write(buffer, length);

		current = start;
		buffer[0] = 0;

		for (int lane = 0; lane < 3; ++lane)
			if (m_vocals[lane] && m_vocals[lane]->save_pitch_bch(lane + 1, current))
				++buffer[0];

		if (buffer[0] > 0)
		{
			numEvents = 2;
			outFile.put(0);
			// Event type - Vocalize
			outFile.put(10);
			length = uint32_t(current - buffer);
			length.writeToFile(outFile);
			outFile.write(buffer, length);
		}
	}
	return numEvents;
}
