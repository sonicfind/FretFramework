#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include "Tracks\InstrumentalTracks\DrumTrack\DrumTrackConverter.h"
#include <iostream>

void Song::loadFile_Cht()
{
	if (m_ini.m_eighthnote_hopo)
		m_ini.m_hopo_frequency.setDefault(96);
	else
		m_ini.m_hopo_frequency.setDefault(64);

	m_ini.m_sustain_cutoff_threshold.setDefault(64);
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	m_version_cht = 1;
	// Loads the file into a char array and traverses it byte by byte
	// or by skipping to a new line character
	TextTraversal traversal(m_filepath);
	DrumNote_Legacy::resetLaning();
	InstrumentalTrack<DrumNote_Legacy> drumsLegacy("null", -1);
	do
	{
		if (traversal == '}')
			traversal.next();

		if (traversal != '[')
			continue;

		const char* const trackName = traversal.getCurrent();
		traversal.next();

		if (traversal == '{')
			traversal.next();

		if (strncmp(trackName, "[Song]", 6) == 0)
		{
			if (!m_ini.wasLoaded())
			{
				while (traversal && traversal != '}' && traversal != '[')
				{
					// Utilize short circuiting to stop if a read was valid
					m_version_cht.read(traversal) ||

						m_songInfo.name.read(traversal) ||
						m_songInfo.artist.read(traversal) ||
						m_songInfo.charter.read(traversal) ||
						m_songInfo.album.read(traversal) ||
						m_songInfo.year.read(traversal) ||
						m_songInfo.genre.read(traversal) ||

						m_offset.read(traversal) ||
						m_tickrate.read(traversal) ||

						m_songInfo.difficulty.read(traversal) ||
						m_songInfo.preview_start_time.read(traversal) ||
						m_songInfo.preview_end_time.read(traversal) ||

						m_audioStreams.music.read(traversal) ||
						m_audioStreams.guitar.read(traversal) ||
						m_audioStreams.bass.read(traversal) ||
						m_audioStreams.rhythm.read(traversal) ||
						m_audioStreams.keys.read(traversal) ||
						m_audioStreams.drum.read(traversal) ||
						m_audioStreams.drum_2.read(traversal) ||
						m_audioStreams.drum_3.read(traversal) ||
						m_audioStreams.drum_4.read(traversal) ||
						m_audioStreams.vocals.read(traversal) ||
						m_audioStreams.crowd.read(traversal);
					traversal.next();
				}

				if (!m_songInfo.year.m_value.empty() && m_songInfo.year.m_value[0] == ',')
				{
					auto iter = m_songInfo.year.m_value.begin() + 1;
					while (iter != m_songInfo.year.m_value.end() && *iter == ' ')
						++iter;
					m_songInfo.year.m_value.erase(m_songInfo.year.m_value.begin(), iter);
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
			else
			{
				m_songInfo.name = m_ini.m_name;
				m_songInfo.artist = m_ini.m_artist;
				m_songInfo.charter = m_ini.m_charter;
				m_songInfo.album = m_ini.m_album;
				m_songInfo.year = m_ini.m_year;
				m_songInfo.genre = m_ini.m_genre;
				m_offset = m_ini.m_delay;

				m_songInfo.preview_start_time = m_ini.m_preview_start_time;
				m_songInfo.preview_end_time = m_ini.m_preview_end_time;

				m_songInfo.difficulty = m_ini.m_diff_band;

				while (traversal && traversal != '}' && traversal != '[')
				{
					// Utilize short circuiting to stop if a read was valid
					m_version_cht.read(traversal) ||

						m_tickrate.read(traversal) ||
						m_audioStreams.music.read(traversal) ||
						m_audioStreams.guitar.read(traversal) ||
						m_audioStreams.bass.read(traversal) ||
						m_audioStreams.rhythm.read(traversal) ||
						m_audioStreams.keys.read(traversal) ||
						m_audioStreams.drum.read(traversal) ||
						m_audioStreams.drum_2.read(traversal) ||
						m_audioStreams.drum_3.read(traversal) ||
						m_audioStreams.drum_4.read(traversal) ||
						m_audioStreams.vocals.read(traversal) ||
						m_audioStreams.crowd.read(traversal);
					traversal.next();
				}
			}

			// Sets the threshold for forcing guitar notes and for sustains
			// Automatically sets the threshold to either 1/3 or 1/2 of the tickrate if they are at the default value
			if (m_ini.m_eighthnote_hopo)
				m_ini.m_hopo_frequency.setDefault(m_tickrate / 2);
			else
				m_ini.m_hopo_frequency.setDefault(m_tickrate / 3);

			m_ini.m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
			Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
			Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);
		}
		else if (strncmp(trackName, "[SyncTrack]", 11) == 0)
		{
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position;
				if (traversal.extract(position))
				{
					// Ensures ascending order
					if (m_sync.back().first <= position)
					{
						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < position)
						{
							static SyncValues prev;
							prev = m_sync.back().second;
							m_sync.push_back({ position, prev });
						}

						traversal.skipEqualsSign();

						if (strncmp(traversal.getCurrent(), "TS", 2) == 0)
						{
							traversal.move(2);
							uint32_t numerator = 4, denom = 2;
							if (traversal.extract(numerator))
							{
								traversal.extract(denom);
								m_sync.back().second.setTimeSig(numerator, denom);
							}
						}
						else
						{
							switch (traversal.extract())
							{
							case 'b':
							case 'B':
							{
								uint32_t bpm = 120000;
								if (traversal.extract(bpm))
									m_sync.back().second.setBPM(bpm * .001f);
							}
								break;
							case 'a':
							case 'A':
							{
								uint32_t anchor = 0;
								if (traversal.extract(anchor))
									m_sync.back().second.setAnchor(anchor);
							}
							}
						}
					}
				}
				
				traversal.next();
			}
		}
		else if (strncmp(trackName, "[Events]", 8) == 0)
		{
			// If reading version 1.X of the .chart format, construct the vocal track from this list
			uint32_t phrase = 0;
			bool phraseActive = false;
			uint32_t prevPosition = 0;
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position;
				if (traversal.extract(position))
				{
					// Ensures ascending order
					if (prevPosition <= position)
					{
						prevPosition = position;
						// Skip '='
						traversal.skipEqualsSign();

						if (strncmp(traversal.getCurrent(), "SE", 2) == 0)
						{
							traversal.move(2);
							std::string_view ev = traversal.extractText();
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
								m_sectionMarkers.push_back({ position, std::string(ev) });
						}
						else if (traversal.extract() == 'E')
						{
							std::string_view ev = traversal.extractText();
							if (strncmp(ev.data(), "section", 7) == 0)
							{
								if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
									m_sectionMarkers.push_back({ position, std::string(ev.substr(8)) });
								goto NextLine;
							}
							else if (m_version_cht < 2)
							{
								VocalTrack<1>* vocals = reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9]);
								if (strncmp(ev.data(), "lyric", 5) == 0)
									vocals->addLyric(0, position, std::string(ev.substr(6)));
								else if (strncmp(ev.data(), "phrase_start", 12) == 0)
								{
									if (phraseActive)
										vocals->addPhrase(phrase, new LyricLine(position - phrase));
									phrase = position;
									phraseActive = true;
								}
								else if (strncmp(ev.data(), "phrase_end", 10) == 0)
								{
									vocals->addPhrase(phrase, new LyricLine(position - phrase));
									phraseActive = false;
								}
								else
									goto WriteAsGlobalEvent;
								goto NextLine;
							}

						WriteAsGlobalEvent:
							if (m_globalEvents.empty() || m_globalEvents.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::string>> pairNode;
								pairNode.first = position;
								m_globalEvents.push_back(pairNode);
							}

							m_globalEvents.back().second.push_back(std::string(ev));
						}
					}
				}

			NextLine:
				traversal.next();
			}
		}
		else if (m_version_cht > 1)
		{
			if (strncmp(trackName, "[LeadGuitar]", 12) == 0)
				s_noteTracks[0]->load_cht(traversal);
			else if (strncmp(trackName, "[LeadGuitar_GHL]", 16) == 0)
				s_noteTracks[1]->load_cht(traversal);
			else if (strncmp(trackName, "[BassGuitar]", 12) == 0)
				s_noteTracks[2]->load_cht(traversal);
			else if (strncmp(trackName, "[BassGuitar_GHL]", 16) == 0)
				s_noteTracks[3]->load_cht(traversal);
			else if (strncmp(trackName, "[RhythmGuitar]", 14) == 0)
				s_noteTracks[4]->load_cht(traversal);
			else if (strncmp(trackName, "[CoopGuitar]", 12) == 0)
				s_noteTracks[5]->load_cht(traversal);
			else if (strncmp(trackName, "[Keys]", 6) == 0)
				s_noteTracks[6]->load_cht(traversal);
			else if (strncmp(trackName, "[Drums_4Lane]", 13) == 0)
				s_noteTracks[7]->load_cht(traversal);
			else if (strncmp(trackName, "[Drums_5Lane]", 13) == 0)
				s_noteTracks[8]->load_cht(traversal);
			else if (strncmp(trackName, "[Vocals]", 8) == 0)
				s_noteTracks[9]->load_cht(traversal);
			else if (strncmp(trackName, "[Harmonies]", 11) == 0)
				s_noteTracks[10]->load_cht(traversal);
			else
				traversal.skipTrack();
		}
		else
		{
			Instrument ins = Instrument::None;
			if (strstr(trackName, "Single"))
				ins = Instrument::Guitar_lead;
			else if (strstr(trackName, "DoubleGuitar"))
				ins = Instrument::Guitar_coop;
			else if (strstr(trackName, "DoubleBass"))
				ins = Instrument::Guitar_bass;
			else if (strstr(trackName, "DoubleRhythm"))
				ins = Instrument::Guitar_rhythm;
			else if (strstr(trackName, "Drums"))
			{
				if (!m_ini.m_five_lane_drums.isActive())
					ins = Instrument::Drums_Legacy;
				else if (!m_ini.m_five_lane_drums)
					ins = Instrument::Drums_4;
				else
					ins = Instrument::Drums_5;
			}
			else if (strstr(trackName, "Keys"))
				ins = Instrument::Keys;
			else if (strstr(trackName, "GHLGuitar"))
				ins = Instrument::Guitar_lead_6;
			else if (strstr(trackName, "GHLBass"))
				ins = Instrument::Guitar_bass_6;

			int difficulty = -1;
			if (strstr(trackName, "Expert"))
				difficulty = 3;
			else if (strstr(trackName, "Hard"))
				difficulty = 2;
			else if (strstr(trackName, "Medium"))
				difficulty = 1;
			else if (strstr(trackName, "Easy"))
				difficulty = 0;

			if (ins != Instrument::None && difficulty != -1)
			{
				switch (ins)
				{
				case Instrument::Guitar_lead:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_lead_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_rhythm:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_coop:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Keys:
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_Legacy:
					drumsLegacy.load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_5:
					reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->load_chart_V1(difficulty, traversal);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	}
	while (traversal.next());

	if (drumsLegacy.occupied())
	{
		if (DrumNote_Legacy::isFiveLane())
			DrumTrackConverter::convert(drumsLegacy, reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8]));
		else
			DrumTrackConverter::convert(drumsLegacy, reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7]));
	}
	m_version_cht = 2;
}

void Song::saveFile_Cht(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc);
	outFile << "[Song]\n{\n";
	m_version_cht.write(outFile);
	m_songInfo.name.write(outFile);
	m_songInfo.artist.write(outFile);
	m_songInfo.charter.write(outFile);
	m_songInfo.album.write(outFile);
	m_songInfo.year.write(outFile);

	m_offset.write(outFile);
	m_tickrate.write(outFile);

	m_songInfo.difficulty.write(outFile);
	m_songInfo.preview_start_time.write(outFile);
	m_songInfo.preview_end_time.write(outFile);
	m_songInfo.genre.write(outFile);

	m_audioStreams.music.write(outFile);
	m_audioStreams.guitar.write(outFile);
	m_audioStreams.bass.write(outFile);
	m_audioStreams.rhythm.write(outFile);
	m_audioStreams.keys.write(outFile);
	m_audioStreams.drum.write(outFile);
	m_audioStreams.drum_2.write(outFile);
	m_audioStreams.drum_3.write(outFile);
	m_audioStreams.drum_4.write(outFile);
	m_audioStreams.vocals.write(outFile);
	m_audioStreams.crowd.write(outFile);
	outFile << "}\n";

	outFile << "[SyncTrack]\n{\n";
	for (const auto& sync : m_sync)
		sync.second.writeSync_cht(sync.first, outFile);
	outFile << "}\n";

	outFile << "[Events]\n{\n";
	auto sectIter = m_sectionMarkers.begin();
	for (auto eventIter = m_globalEvents.begin(); eventIter != m_globalEvents.end(); ++eventIter)
	{
		while (sectIter != m_sectionMarkers.end() && sectIter->first <= eventIter->first)
		{
			outFile << '\t' << sectIter->first << " = SE \"" << sectIter->second << "\"\n";
			++sectIter;
		}

		for (const auto& str : eventIter->second)
			outFile << '\t' << eventIter->first << " = E \"" << str << "\"\n";
	}

	while (sectIter != m_sectionMarkers.end())
	{
		outFile << '\t' << sectIter->first << " = SE \"" << sectIter->second << "\"\n";
		++sectIter;
	}
	outFile << "}\n";

	for (const NoteTrack* const track : s_noteTracks)
		track->save_cht(outFile);
	outFile.close();
}
