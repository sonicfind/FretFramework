#pragma once
#include "BasicTrack.h"
#include "Difficulty/Difficulty_cht.hpp"

template <class T>
void BasicTrack<T>::load_cht(std::fstream& inFile)
{
	static char buffer[512] = { 0 };
	try
	{
		while (inFile.getline(buffer, 512) && buffer[0] != '}')
		{
			// Will default to adding to the BRE difficulty if the difficulty name can't be matched
			int i = 0;
			while (i < 4 && !strstr(buffer, m_difficulties[i].m_name))
				++i;

			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			m_difficulties[i].load_cht(inFile);
		}

		if (!inFile)
			throw EndofFileException();
	}
	catch (EndofTrackException EOT)
	{
		std::cout << "Error: Parsing of track " << m_name << " ended improperly" << std::endl;
		clear();
	}
	catch (EndofFileException EoF)
	{
		std::cout << "Error in track " << m_name << std::endl;
		throw EndofFileException();
	}
}

template <class T>
void BasicTrack<T>::save_cht(std::fstream& outFile) const
{
	if (occupied())
	{
		outFile << "[" << m_name << "]\n{\n";
		for (int diff = 4; diff >= 0; --diff)
			if (m_difficulties[diff].occupied())
				m_difficulties[diff].save_cht(outFile);
		outFile << "}\n";
	}
}
