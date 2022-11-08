#pragma once
#include "VocalTrack_Scan.h"
#include "NoteExceptions.h"
#include <iostream>

template<int numTracks>
inline void VocalTrack_Scan<numTracks>::scan_bch(BCHTraversal& traversal)
{
	unsigned char expectedScan = 0;
	uint32_t vocalPhraseEnd = 0;
	bool checked[numTracks]{};

	if (!traversal.canParseNewChunk() || !traversal.validateChunk("LYRC"))
		goto ValidateAnim;
	else if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("ANIM") && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
	{
		// Sets the next track to whatever next valid track comes first, if any exist

		const unsigned char* const anim = traversal.findNextChunk("ANIM");
		const unsigned char* const inst = traversal.findNextChunk("INST");
		const unsigned char* const vocl = traversal.findNextChunk("VOCL");
		if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
			traversal.setNextTrack(anim);
		else if (inst && (!vocl || inst < vocl))
			traversal.setNextTrack(inst);
		else
			traversal.setNextTrack(vocl);
	}

	expectedScan = traversal.extract<unsigned char>();
	if (expectedScan >= 8)
		goto ValidateAnim;

	while (traversal.next())
	{
		try
		{
			if (traversal.getEventType() == 9)
			{
				if (traversal.getPosition() < vocalPhraseEnd)
				{
					unsigned char lane = traversal.extract<unsigned char>();
					if (0 < lane && lane <= numTracks)
					{
						--lane;
						if (!checked[lane] && Vocal::isEventPlayable(traversal))
						{
							if (expectedScan == 0)
							{
								m_scanValue = 8;
								goto ValidateAnim;
							}
							else
							{
								checked[lane] = true;
								m_scanValue |= 1 << lane;

								if (m_scanValue == expectedScan)
									goto ValidateAnim;
							}
						}
					}
				}
			}
			else if (traversal.getEventType() == 5)
			{
				const unsigned char phrase = traversal.extract<unsigned char>();
				if (phrase == 4)
					vocalPhraseEnd = traversal.getPosition() + traversal.extractWebType();
			}
		}
		catch (...)
		{
		}
	}

ValidateAnim:
	traversal.skipTrack();
	if (traversal.canParseNewChunk() && traversal.validateChunk("ANIM"))
	{
		if (traversal.doesNextTrackExist() && !traversal.checkNextChunk("INST") && !traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}
		traversal.skipTrack();
	}
}
