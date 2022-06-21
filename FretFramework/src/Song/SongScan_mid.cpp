#include "SongScan.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"

void SongScan::scanFile_Midi()
{
	MidiTraversal traversal(m_filepath);
	s_fileHasher.addNode(m_hash, traversal);

	while (traversal)
	{
		// Checks for a chunk header
		if (traversal.validateChunk())
		{
			if (traversal.next() && traversal.getEventType() < 128 && traversal.getEventType() != 0x2F)
			{
				std::string name;
				if (traversal.getEventType() == 3)
					name = traversal.extractText();

				// SyncTrack
				if (traversal.getTrackNumber() != 1 && name != "EVENTS")
				{
					if (name == "PART GUITAR" || name == "T1 GEMS")
						reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->scan_midi(traversal, m_noteTrackScans[0]);
					else if (name == "PART GUITAR GHL")
						reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->scan_midi(traversal, m_noteTrackScans[1]);
					else if (name == "PART BASS")
						reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->scan_midi(traversal, m_noteTrackScans[2]);
					else if (name == "PART BASS GHL")
						reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->scan_midi(traversal, m_noteTrackScans[3]);
					else if (name == "PART RHYTHM")
						reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->scan_midi(traversal, m_noteTrackScans[4]);
					else if (name == "PART GUITAR COOP")
						reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->scan_midi(traversal, m_noteTrackScans[5]);
					else if (name == "PART KEYS")
						reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->scan_midi(traversal, m_noteTrackScans[6]);
					else if (name == "PART DRUMS")
					{
						if (!m_ini.m_five_lane_drums.isActive())
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
						else if (!m_ini.m_five_lane_drums)
							reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->scan_midi(traversal, m_noteTrackScans[7]);
						else
							reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->scan_midi(traversal, m_noteTrackScans[8]);
					}
					else if (name == "PART VOCALS")
						reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9])->scan_midi(0, traversal, m_noteTrackScans[9]);
					else if (name == "HARM1")
						reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->scan_midi(0, traversal, m_noteTrackScans[10]);
					else if (name == "HARM2")
						reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->scan_midi(1, traversal, m_noteTrackScans[10]);
					else if (name == "HARM3")
						reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->scan_midi(2, traversal, m_noteTrackScans[10]);
				}
			}
		}
		else
			traversal.setNextTrack(traversal.findNextChunk());
		traversal.skipTrack();
	}
}
