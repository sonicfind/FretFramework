#include "Song.h"

void Song::scanFile_Bch(bool multiThreaded)
{
	BCHTraversal traversal(m_fullPath);
	if (multiThreaded)
		traversal.addMD5toThreadQueue(m_hash);

	m_version_bch = traversal.extractU16();
	traversal.move(2);
	traversal.move(2);
	while (traversal)
	{
		if (traversal.validateChunk("INST") || traversal.validateChunk("VOCL"))
		{
			// Instrument ID
			const unsigned char ID = traversal.getTrackID();
			if (ID < 11)
			{
				try
				{
					s_noteTracks[ID]->scan_bch(traversal, m_noteTrackScans[ID]);
				}
				catch (std::runtime_error err)
				{
					traversal.skipTrack();
				}
			}
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

	if (!multiThreaded)
	{
		if (!isValid())
			throw std::runtime_error(": No notes found");
		traversal.hashMD5(m_hash);
	}
}
