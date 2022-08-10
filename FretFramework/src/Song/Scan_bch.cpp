#include "Song.h"

void Song::scanFile(BCHTraversal&& traversal)
{
	m_version_bch = traversal.extractU16();
	traversal.move(4);
	while (traversal.canParseNewChunk())
	{
		if (traversal.validateChunk("INST") || traversal.validateChunk("VOCL"))
		{
			// Instrument ID
			const unsigned char ID = traversal.getTrackID();
			if (ID < 11)
				s_noteTracks.trackArray[ID]->scan_bch(traversal, m_noteTrackScans[ID]);
			else
				traversal.skipTrack();
		}
		else if (traversal.validateChunk("SYNC") || traversal.validateChunk("EVTS"))
			traversal.skipTrack();
		else
		{
			const unsigned char* const inst = traversal.findNextChunk("INST");
			const unsigned char* const vocl = traversal.findNextChunk("VOCL");
			if (inst && (!vocl || inst < vocl))
				traversal.setNextTrack(inst);
			else
				traversal.setNextTrack(vocl);
			traversal.skipTrack();
		}
	}
}
