#include "Song.h"

void Song::scanFile(TextTraversal&& traversal)
{
	m_version_cht = 1;
	InstrumentalTrack_Scan<DrumNote_Legacy>* drumsLegacy_scan = nullptr;
	do
	{
		if (traversal != '[')
		{
			traversal.next();
			continue;
		}

		traversal.next();

		if (traversal == '{')
			traversal.next();

		if (traversal.isTrackName("[Song]"))
		{
			if (m_ini.wasLoaded())
			{
				while (traversal && traversal != '}' && traversal != '[')
				{
					try
					{
						if (traversal.extractModifierName() == "FileVersion")
						{
							m_version_cht.read(traversal);
							traversal.skipTrack();
							break;
						}
					}
					catch (...) {}
					traversal.next();
				}
			}
			else
			{
				traverseCHTSongSection(this, traversal, SongModMapping::s_SONGINFOMAP);

				if (!m_songInfo.year.m_string->empty() && m_songInfo.year.m_string[0] == ',')
				{
					auto iter = m_songInfo.year.m_string->begin() + 1;
					while (iter != m_songInfo.year.m_string->end() && *iter == ' ')
						++iter;
					m_songInfo.year.m_string->erase(m_songInfo.year.m_string->begin(), iter);
				}

				m_ini.setBaseModifiers();

				*m_ini.getName() = &m_songInfo.name;
				*m_ini.getArtist() = &m_songInfo.artist;
				*m_ini.getCharter() = &m_songInfo.charter;
				*m_ini.getAlbum() = &m_songInfo.album;
				*m_ini.getYear() = &m_songInfo.year;
				*m_ini.getGenre() = &m_songInfo.genre;

				if (m_songInfo.preview_start_time)
					m_ini.setModifier<NumberModifier<float>>("preview_start_time", m_songInfo.preview_start_time);
				if (m_songInfo.preview_end_time)
					m_ini.setModifier<NumberModifier<float>>("preview_end_time", m_songInfo.preview_end_time);
				if (m_songInfo.difficulty)
					m_ini.setModifier<NumberModifier<int16_t>>("diff_band", m_songInfo.difficulty);
			}
		}
		else if (traversal.isTrackName("[SyncTrack]") || traversal.isTrackName("[Events]"))
			traversal.skipTrack();
		else if (m_version_cht > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
				++i;

			if (i < 11)
				s_noteTracks[i]->scan_cht(traversal, m_noteTrackScans[i]);
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
				if (BooleanModifier* fiveLaneDrums = m_ini.getModifier<BooleanModifier>("five_lane_drums"))
				{
					if (fiveLaneDrums->m_boolean)
						ins = Instrument::Drums_5;
					else
						ins = Instrument::Drums_4;
				}
				else if (drumsLegacy_scan && drumsLegacy_scan->isFiveLane())
					ins = Instrument::Drums_5;
				else
					ins = Instrument::Drums_Legacy;
						
			}
			else if (traversal.cmpTrackName("Keys]"))
				ins = Instrument::Keys;
			else if (traversal.cmpTrackName("GHLGuitar]"))
				ins = Instrument::Guitar_lead_6;
			else if (traversal.cmpTrackName("GHLBass]"))
				ins = Instrument::Guitar_bass_6;

			if (ins != Instrument::None && difficulty != -1)
			{
				switch (ins)
				{
				case Instrument::Guitar_lead:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[0]);
					break;
				case Instrument::Guitar_lead_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[1]);
					break;
				case Instrument::Guitar_bass:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[2]);
					break;
				case Instrument::Guitar_bass_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[3]);
					break;
				case Instrument::Guitar_rhythm:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[4]);
					break;
				case Instrument::Guitar_coop:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[5]);
					break;
				case Instrument::Keys:
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[6]);
					break;
				case Instrument::Drums_Legacy:
					if (drumsLegacy_scan == nullptr)
						drumsLegacy_scan = new InstrumentalTrack_Scan<DrumNote_Legacy>;
					drumsLegacy_scan->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[7]);
					break;
				case Instrument::Drums_5:
					reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get())->scan_chart_V1(difficulty, traversal, m_noteTrackScans[8]);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	} while (traversal);

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
}
