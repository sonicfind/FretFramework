#include "Song.h"

void Song::scanFile_Cht(bool multiThreaded)
{
	m_version_cht = 1;
	TextTraversal traversal(m_fullPath);
	if (multiThreaded)
		traversal.addMD5toThreadQueue(m_hash);

	InstrumentalTrack_Scan<DrumNote_Legacy>* drumsLegacy_scan = nullptr;
	do
	{
		if (traversal == '}')
			traversal.next();

		if (traversal != '[')
			continue;

		traversal.next();

		if (traversal.isTrackName("[Song]"))
		{
			if (traversal == '{')
				traversal.next();

			if (m_ini.wasLoaded())
			{
				while (traversal && traversal != '}' && traversal != '[')
				{
					try
					{
						if (m_version_cht.read(traversal))
						{
							// Skip rest of data
							while (traversal && traversal != '}' && traversal != '[')
								traversal.next();
							break;
						}
					}
					catch (std::runtime_error err)
					{
						std::cout << "Line " << traversal.getLineNumber() << ": " << err.what() << std::endl;
					}
					traversal.next();
				}
			}
			else
			{
				while (traversal && traversal != '}' && traversal != '[')
				{
					try
					{
						// Utilize short circuiting to stop if a read was valid
						// Just need to pull out data that can be written to an ini file after the scan
						m_version_cht.read(traversal) ||

							m_songInfo.name.read(traversal) ||
							m_songInfo.artist.read(traversal) ||
							m_songInfo.charter.read(traversal) ||
							m_songInfo.album.read(traversal) ||
							m_songInfo.year.read(traversal) ||
							m_songInfo.genre.read(traversal) ||

							m_offset.read(traversal) ||

							m_songInfo.difficulty.read(traversal) ||
							m_songInfo.preview_start_time.read(traversal) ||
							m_songInfo.preview_end_time.read(traversal);
					}
					catch (std::runtime_error err)
					{
						std::cout << "Line " << traversal.getLineNumber() << ": " << err.what() << std::endl;
					}
					traversal.next();
				}

				if (!m_songInfo.year.m_value->empty() && m_songInfo.year.m_value[0] == ',')
				{
					auto iter = m_songInfo.year.m_value->begin() + 1;
					while (iter != m_songInfo.year.m_value->end() && *iter == ' ')
						++iter;
					m_songInfo.year.m_value->erase(m_songInfo.year.m_value->begin(), iter);
				}

				m_ini.m_name = m_songInfo.name;
				m_ini.m_artist = m_songInfo.artist;
				m_ini.m_charter = m_songInfo.charter;
				m_ini.m_album = m_songInfo.album;
				m_ini.m_year = m_songInfo.year;
				m_ini.m_genre = m_songInfo.genre;
				m_ini.m_delay = m_offset;

				m_ini.m_preview_start_time = m_songInfo.preview_start_time;
				m_ini.m_preview_end_time = m_songInfo.preview_end_time;

				m_ini.m_diff_band = m_songInfo.difficulty;
			}
		}
		else if (traversal.isTrackName("[SyncTrack]") || traversal.isTrackName("[Events]"))
		{
			if (traversal == '{')
				traversal.next();

			while (traversal && traversal != '}' && traversal != '[')
				traversal.next();
		}
		else if (m_version_cht > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
				++i;

			if (i < 11)
			{
				if (traversal == '{')
					traversal.next();

				s_noteTracks[i]->scan_cht(traversal, m_noteTrackScans[i]);
			}
			else
				traversal.skipTrack();
		}
		else
		{
			int difficulty = -1;
			if (traversal.cmpTrackName("[Expert"))
				difficulty = 3;
			else if (traversal.cmpTrackName("[Hard"))
				difficulty = 2;
			else if (traversal.cmpTrackName("[Medium"))
				difficulty = 1;
			else if (traversal.cmpTrackName("[Easy"))
				difficulty = 0;

			Instrument ins = Instrument::None;
			if (traversal.cmpTrackName("Single]"))
				ins = Instrument::Guitar_lead;
			else if (traversal.cmpTrackName("DoubleGuitar]"))
				ins = Instrument::Guitar_coop;
			else if (traversal.cmpTrackName("DoubleBass]"))
				ins = Instrument::Guitar_bass;
			else if (traversal.cmpTrackName("DoubleRhythm]"))
				ins = Instrument::Guitar_rhythm;
			else if (traversal.cmpTrackName("Drums]"))
			{
				if (!m_ini.m_five_lane_drums.isActive() && (!drumsLegacy_scan || !drumsLegacy_scan->isFiveLane()))
					ins = Instrument::Drums_Legacy;
				else if (m_ini.m_five_lane_drums || (drumsLegacy_scan && drumsLegacy_scan->isFiveLane()))
					ins = Instrument::Drums_5;
				else
					ins = Instrument::Drums_4;	
			}
			else if (traversal.cmpTrackName("Keys]"))
				ins = Instrument::Keys;
			else if (traversal.cmpTrackName("GHLGuitar]"))
				ins = Instrument::Guitar_lead_6;
			else if (traversal.cmpTrackName("GHLBass]"))
				ins = Instrument::Guitar_bass_6;

			if (ins != Instrument::None && difficulty != -1)
			{
				if (traversal == '{')
					traversal.next();

				switch (ins)
				{
				case Instrument::Guitar_lead:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[0]);
					break;
				case Instrument::Guitar_lead_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[1]);
					break;
				case Instrument::Guitar_bass:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[2]);
					break;
				case Instrument::Guitar_bass_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[3]);
					break;
				case Instrument::Guitar_rhythm:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[4]);
					break;
				case Instrument::Guitar_coop:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[5]);
					break;
				case Instrument::Keys:
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[6]);
					break;
				case Instrument::Drums_Legacy:
					if (drumsLegacy_scan == nullptr)
						drumsLegacy_scan = new InstrumentalTrack_Scan<DrumNote_Legacy>;
					drumsLegacy_scan->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[7]);
					break;
				case Instrument::Drums_5:
					reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->scan_chart_V1(difficulty, traversal, m_noteTrackScans[8]);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	} while (traversal.next());

	if (drumsLegacy_scan)
	{
		if (drumsLegacy_scan->getValue() > 0)
		{
			if (!drumsLegacy_scan->isFiveLane())
				m_noteTrackScans[7] = std::make_unique<InstrumentalTrack_Scan<DrumNote<4, DrumPad_Pro>>>();
			else
				m_noteTrackScans[8] = std::make_unique<InstrumentalTrack_Scan<DrumNote<5, DrumPad>>>();
			m_noteTrackScans[7 + drumsLegacy_scan->isFiveLane()]->addFromValue(drumsLegacy_scan->getValue());
		}
		delete drumsLegacy_scan;
	}

	if (!multiThreaded)
	{
		if (!isValid())
			throw std::runtime_error(": No notes found");
		traversal.hashMD5(m_hash);
	}
}
