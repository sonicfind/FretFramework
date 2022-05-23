#pragma once
#include "InstrumentalTrack.h"
#include "Difficulty/Difficulty_cht.hpp"
#include "FileTraversal/TextFileTraversal.h"

template<class T>
inline void InstrumentalTrack<T>::load_chart_V1(int diff, TextTraversal& traversal)
{
	m_difficulties[diff].load_chart_V1(traversal);
}

template <class T>
inline void InstrumentalTrack<T>::load_cht(TextTraversal& traversal)
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
inline void InstrumentalTrack<T>::save_cht(std::fstream& outFile) const
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