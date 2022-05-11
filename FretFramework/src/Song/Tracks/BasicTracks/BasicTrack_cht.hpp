#pragma once
#include "BasicTrack.h"
#include "Difficulty/Difficulty_cht.hpp"
#include "..\TextFileTraversal.h"

template <class T>
void BasicTrack<T>::load_cht(TextTraversal& traversal)
{
	while (traversal && traversal != '}')
	{
		if (traversal == '[')
		{
			int i = 0;
			while (i < 5 && strncmp(traversal.getCurrent(), m_difficulties[i].m_name.data(), m_difficulties[i].m_name.length()) != 0)
				++i;

			traversal.next();

			if (traversal == '{')
				traversal.next();

			if (i < 5)
				m_difficulties[i].load_cht(traversal);
			else
				traversal.skipTrack();

			if (traversal == '}')
				traversal.next();
		}
		else if (traversal == '{')
		{
			traversal.next();
			traversal.skipTrack();
		}
		else
			traversal.next();
	}
}

template <class T>
void BasicTrack<T>::save_cht(std::fstream& outFile) const
{
	if (occupied())
	{
		outFile << m_name << "\n{\n";
		for (int diff = 4; diff >= 0; --diff)
			if (m_difficulties[diff].occupied())
				m_difficulties[diff].save_cht(outFile);
		outFile << "}\n";
	}
}
