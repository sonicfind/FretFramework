#include "SongEntry.h"
#include "Tracks/Scans/VocalTracks/VocalTrack_Scan_midi.hpp"

void SongEntry::scanFile(MidiTraversal&& traversal)
{
	while (traversal)
	{
		// Checks for a chunk header
		if (traversal.validateChunk())
		{
			if (traversal.getTrackNumber() > 1)
			{
				const std::string& name = traversal.getTrackName();
				if (name == "PART GUITAR" || name == "T1 GEMS")
					m_noteTrackScans.lead_5.scan_midi(traversal);
				else if (name == "PART GUITAR GHL")
					m_noteTrackScans.lead_6.scan_midi(traversal);
				else if (name == "PART BASS")
					m_noteTrackScans.bass_5.scan_midi(traversal);
				else if (name == "PART BASS GHL")
					m_noteTrackScans.bass_6.scan_midi(traversal);
				else if (name == "PART RHYTHM")
					m_noteTrackScans.rhythm.scan_midi(traversal);
				else if (name == "PART GUITAR COOP")
					m_noteTrackScans.coop.scan_midi(traversal);
				else if (name == "PART KEYS")
					m_noteTrackScans.keys.scan_midi(traversal);
				else if (name == "PART DRUMS")
				{
					if (TxtFileModifier* fiveLaneDrums = getModifier("five_lane_drums"))
					{
						if (fiveLaneDrums->getValue<bool>())
							m_noteTrackScans.drums4_pro.scan_midi(traversal);
						else
							m_noteTrackScans.drums5.scan_midi(traversal);
					}
					else
					{
						InstrumentalTrack_Scan<DrumNote_Legacy> drumScan_legacy;
						drumScan_legacy.scan_midi(traversal);
						m_noteTrackScans.scanArray[7 + drumScan_legacy.isFiveLane()]->addFromValue(drumScan_legacy.getValue());
					}
				}
				else if (name == "PART VOCALS")
					m_noteTrackScans.vocals.scan_midi<0>(traversal);
				else if (name == "HARM1")
					m_noteTrackScans.harmonies.scan_midi<0>(traversal);
				else if (name == "HARM2")
					m_noteTrackScans.harmonies.scan_midi<1>(traversal);
				else if (name == "HARM3")
					m_noteTrackScans.harmonies.scan_midi<2>(traversal);
			}
		}
		else
			traversal.setNextTrack(traversal.findNextChunk());
		traversal.skipTrack();
	}
}
