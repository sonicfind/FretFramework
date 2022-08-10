#include "Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"

void Song::scanFile(MidiTraversal&& traversal)
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
					s_noteTracks.lead_5.scan_midi(traversal, m_noteTrackScans[0]);
				else if (name == "PART GUITAR GHL")
					s_noteTracks.lead_6.scan_midi(traversal, m_noteTrackScans[1]);
				else if (name == "PART BASS")
					s_noteTracks.bass_5.scan_midi(traversal, m_noteTrackScans[2]);
				else if (name == "PART BASS GHL")
					s_noteTracks.bass_6.scan_midi(traversal, m_noteTrackScans[3]);
				else if (name == "PART RHYTHM")
					s_noteTracks.rhythm.scan_midi(traversal, m_noteTrackScans[4]);
				else if (name == "PART GUITAR COOP")
					s_noteTracks.coop.scan_midi(traversal, m_noteTrackScans[5]);
				else if (name == "PART KEYS")
					s_noteTracks.keys.scan_midi(traversal, m_noteTrackScans[6]);
				else if (name == "PART DRUMS")
				{
					if (TxtFileModifier* fiveLaneDrums = getModifier("five_lane_drums"))
					{
						if (fiveLaneDrums->getValue<bool>())
							s_noteTracks.drums4_pro.scan_midi(traversal, m_noteTrackScans[7]);
						else
							s_noteTracks.drums5.scan_midi(traversal, m_noteTrackScans[8]);
					}
					else
					{
						InstrumentalTrack_Scan<DrumNote_Legacy> drumScan_legacy;
						drumScan_legacy.scan_midi(traversal);
						if (drumScan_legacy.getValue() > 0)
						{
							if (!drumScan_legacy.isFiveLane())
								m_noteTrackScans[7] = std::make_unique<InstrumentalTrack_Scan<DrumNote<4, DrumPad_Pro>>>(drumScan_legacy.getValue());
							else
								m_noteTrackScans[8] = std::make_unique<InstrumentalTrack_Scan<DrumNote<5, DrumPad>>>(drumScan_legacy.getValue());
						}
					}
				}
				else if (name == "PART VOCALS")
					s_noteTracks.vocals.scan_midi<0>(traversal, m_noteTrackScans[9]);
				else if (name == "HARM1")
					s_noteTracks.harmonies.scan_midi<0>(traversal, m_noteTrackScans[10]);
				else if (name == "HARM2")
					s_noteTracks.harmonies.scan_midi<1>(traversal, m_noteTrackScans[10]);
				else if (name == "HARM3")
					s_noteTracks.harmonies.scan_midi<2>(traversal, m_noteTrackScans[10]);
			}
		}
		else
			traversal.setNextTrack(traversal.findNextChunk());
		traversal.skipTrack();
	}
}
