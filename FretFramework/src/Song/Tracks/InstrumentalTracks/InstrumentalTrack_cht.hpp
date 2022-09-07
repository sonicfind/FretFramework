#pragma once
#include "DrumTrack/DrumTrack_Legacy.h"
#include "Difficulty/Difficulty_cht.hpp"
#include "FileTraversal/TextFileTraversal.h"

template<class T>
inline void InstrumentalTrack_Scan<T>::scan_chart_V1(int diff, TextTraversal& traversal)
{
	if (m_difficulties[diff].scan_chart_V1(traversal))
		m_scanValue |= 1 << diff;
}

template <class T>
inline void InstrumentalTrack_Scan<T>::scan_cht(TextTraversal& traversal)
{
	while (traversal && traversal != '}')
	{
		if (traversal == '[')
		{
			traversal.setTrackName();
			int i = 0;
			while (i < 5 && !traversal.isTrackName(m_difficulties[i].m_name))
				++i;

			if (i == 5)
				return;

			traversal.next();

			if (i < 4)
			{
				if (traversal == '{')
					traversal.next();

				if (m_difficulties[i].scan_cht(traversal))
					m_scanValue |= 1 << i;
			}
			else
				traversal.skipTrack();

			if (traversal == '}')
				traversal.next();
		}
		else if (traversal == '{')
		{
			traversal.skipTrack();
			if (traversal == '}')
				traversal.next();
		}
		else
			traversal.next();
	}
}

template<class T>
inline void InstrumentalTrack<T>::scan_chart_V1(int diff, TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<InstrumentalTrack_Scan<T>>();
	reinterpret_cast<InstrumentalTrack_Scan<T>*>(track.get())->scan_chart_V1(diff, traversal);
}

template<class T>
inline void InstrumentalTrack<T>::load_chart_V1(int diff, TextTraversal& traversal)
{
	m_difficulties[diff].load_chart_V1(traversal);
}

template<class T>
inline void InstrumentalTrack<T>::scan_cht(TextTraversal& traversal, std::unique_ptr<NoteTrack_Scan>& track) const
{
	if (track == nullptr)
		track = std::make_unique<InstrumentalTrack_Scan<T>>();
	track->scan_cht(traversal);
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

			if (i == 5)
				return;

			traversal.next();

			if (traversal == '{')
				traversal.next();

			m_difficulties[i].load_cht(traversal);

			if (traversal == '}')
				traversal.next();
		}
		else if (traversal == '{')
		{
			traversal.skipTrack();
			if (traversal == '}')
				traversal.next();
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
