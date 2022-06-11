#pragma once
#include "InstrumentalTrack.h"
#include "Difficulty/Difficulty_cht.hpp"
#include "FileTraversal/TextFileTraversal.h"

template<class T>
inline int InstrumentalTrack<T>::scan_chart_V1(int diff, TextTraversal& traversal)
{
	if (m_difficulties[diff].scan_chart_V1(traversal))
		return 1 << diff;
	return 0;
}

template <class T>
inline int InstrumentalTrack<T>::scan_cht(TextTraversal& traversal)
{
	int ret = 0;
	while (traversal && traversal != '}')
	{
		if (traversal == '[')
		{
			traversal.setTrackName();

			int i = 0;
			// Scanning only takes *playable* notes into account, so BRE can be ignored
			while (i < 4 && !traversal.isTrackName(m_difficulties[i].m_name))
				++i;

			traversal.next();

			if (traversal == '{')
				traversal.next();

			if (i < 4)
			{
				if (m_difficulties[i].scan_cht(traversal))
					ret |= 1 << i;
			}
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

	return ret;
}

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
			traversal.setTrackName();

			int i = 0;
			while (i < 5 && !traversal.isTrackName(m_difficulties[i].m_name))
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
