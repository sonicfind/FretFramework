#pragma once
#include "InstrumentalTrack_Scan.h"
#include "FileTraversal/TextFileTraversal.h"

template<class T>
inline void InstrumentalTrack_Scan<T>::scan_chart_V1(int diff, TextTraversal& traversal)
{
	// End positions to protect from conflicting special phrases
	uint32_t starPowerEnd = 0;
	uint32_t starActivationEnd = 0;

	traversal.resetPosition();
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		try
		{
			traversal.extractPosition();
			unsigned char type = traversal.extract<unsigned char>();

			if (type == 'N' || type == 'n')
			{
				const int lane = traversal.extract<uint32_t>();
				if (traversal.skipInt() && T::testIndex_chartV1(lane))
				{
					m_scanValue |= 1 << diff;
					traversal.skipTrack();
					return;
				}
			}
		}
		catch (...) {}
	} while (traversal.next());
}

constexpr const char* DIFFICULTIES[4] = { "[Easy]", "[Medium]", "[Hard]", "[Expert]" };

template <class T>
inline void InstrumentalTrack_Scan<T>::scan_cht(TextTraversal& traversal)
{
	while (traversal && traversal != '}')
	{
		if (traversal == '[')
		{
			traversal.setTrackName();
			int diff = 0;
			while (diff < 5 && !traversal.isTrackName(DIFFICULTIES[diff]))
				++diff;

			if (diff == 5)
				return;

			traversal.next();

			if (diff < 4)
			{
				if (traversal == '{')
					traversal.next();

				if (scanDifficulty(traversal))
					m_scanValue |= 1 << diff;
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
inline bool InstrumentalTrack_Scan<T>::scanDifficulty(TextTraversal& traversal)
{
	traversal.resetPosition();
	do
	{
		if (traversal == '}' || traversal == '[')
			break;

		unsigned char type;
		try
		{
			traversal.extractPosition();
			type = traversal.extract<unsigned char>();
		}
		catch (...)
		{
			continue;
		}

		switch (type)
		{
		case 'N':
		case 'n':
			if (validate_single<T>(traversal))
				goto Valid;
			break;
		case 'C':
		case 'c':
			if (validate_chord<T>(traversal))
				goto Valid;
			break;
		}

	} while (traversal.next());
	return false;

Valid:
	traversal.skipTrack();
	return true;
}
