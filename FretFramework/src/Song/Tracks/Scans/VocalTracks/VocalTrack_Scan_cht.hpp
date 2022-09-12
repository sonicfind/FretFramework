#pragma once
#include "VocalTrack_Scan.h"
#include "NoteExceptions.h"
#include <iostream>

template<int numTracks>
inline void VocalTrack_Scan<numTracks>::scan_cht(TextTraversal& traversal)
{
	static constexpr int FINALVALUE = (1 << numTracks) - 1;
	bool checks[numTracks]{};
	bool lyricsExist = false;
	uint32_t phraseEnd = 0;

	traversal.resetPosition();
	while (traversal && traversal != '}' && traversal != '[')
	{
		try
		{
			uint32_t position = traversal.extractPosition();
			char type = traversal.extract<unsigned char>();

			// Special Phrases & Text Events are only important for validating proper event order in regards to tick position
			switch (type)
			{
			case 'V':
			case 'v':
			{
				uint32_t lane = traversal.extract<uint32_t>();

				// Only scan for valid vocals
				if (lane == 0 || lane > numTracks || position >= phraseEnd)
					break;

				--lane;
				if (!checks[lane] && traversal.skipLyric())
				{
					lyricsExist = true;

					if (Vocal::isEventPlayable(traversal))
					{
						m_scanValue |= 1 << lane;
						checks[lane] = true;
						if (m_scanValue == FINALVALUE)
						{
							traversal.skipTrack();
							return;
						}
					}
				}
				break;
			}
			case 'P':
			case 'p':
			{
				if (position < phraseEnd && phraseEnd != UINT32_MAX)
					break;

				bool isPhraseStarting = true;
				if (traversal == 'e' || traversal == 'E')
				{
					isPhraseStarting = false;
					traversal.move(1);
				}

				if (traversal == 'h' || traversal == 'H')
					break;

				phraseEnd = isPhraseStarting ? UINT32_MAX : 0;
				break;
			}
			case 'S':
			case 's':
				if (traversal.extract<uint32_t>() == 4 && position >= phraseEnd)
					phraseEnd = position + traversal.extract<uint32_t>();
			}
		}
		catch (...)
		{

		}
		traversal.next();
	}

	if (m_scanValue == 0 && lyricsExist)
		m_scanValue = 8;
}
