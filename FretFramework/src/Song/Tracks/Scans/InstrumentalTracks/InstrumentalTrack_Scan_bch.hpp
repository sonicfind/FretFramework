#pragma once
#include "InstrumentalTrack_Scan.h"

template <class T>
void InstrumentalTrack_Scan<T>::scan_bch(BCHTraversal& traversal)
{
	traversal.move(1);
	while (traversal.canParseNewChunk() && traversal.validateChunk("DIFF"))
	{
		if (traversal.doesNextTrackExist() &&
			!traversal.checkNextChunk("DIFF") &&
			!traversal.checkNextChunk("ANIM") &&
			!traversal.checkNextChunk("INST") &&
			!traversal.checkNextChunk("VOCL"))
		{
			// Sets the next track to whatever next valid track comes first, if any exist

			const unsigned char* const diff = traversal.findNextChunk("DIFF");
			const unsigned char* const anim = traversal.findNextChunk("ANIM");
			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (diff && (!anim || diff < anim) && (!inst || diff < inst) && (!vocl || diff < vocl))
				traversal.setNextTrack(diff);
			else if (anim && (!inst || anim < inst) && (!vocl || anim < vocl))
				traversal.setNextTrack(anim);
			else if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
		}

		unsigned char diff = traversal.getTrackID();
		// Scanning only takes *playable* notes into account, so BRE can be ignored
		if (diff < 4)
		{
			if (scanDifficulty(traversal))
				m_scanValue |= 1 << diff;
		}
		else
			traversal.skipTrack();
	}

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

template<class T>
inline bool InstrumentalTrack_Scan<T>::scanDifficulty(BCHTraversal& traversal)
{
	traversal.move(4);
	while (traversal.next())
	{
		const unsigned char type = traversal.getEventType();
		if (type == 6)
		{
			if (validate_single<T>(traversal))
				goto Valid;
		}
		else if (type == 7)
		{
			if (validate_chord<T>(traversal))
				goto Valid;
		}
	}
	return false;

Valid:
	traversal.skipTrack();
	return true;
}
