#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile(TextTraversal&& traversal)
{
	if (m_ini.m_eighthnote_hopo)
		m_ini.m_hopo_frequency.setDefault(96);
	else
		m_ini.m_hopo_frequency.setDefault(64);

	m_ini.m_sustain_cutoff_threshold.setDefault(64);
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	m_version_cht = 1;
	InstrumentalTrack<DrumNote_Legacy> drumsLegacy;

	while (traversal)
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
			if (!m_ini.wasLoaded())
			{
				while (traversal && traversal != '}' && traversal != '[')
				{
					try
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
					try
					{
						// Utilize short circuiting to stop if a read was valid
						m_version_cht.read(traversal) ||

							(m_offset == 0 && m_offset.read(traversal)) ||
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
					}
					catch (std::runtime_error err)
					{
						std::cout << "Line " << traversal.getLineNumber() << ": " << err.what() << std::endl;
					}
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
		else if (traversal.isTrackName("[SyncTrack]"))
		{
			traversal.resetPosition();
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position = UINT32_MAX;
				try
				{
					position = traversal.extractPosition();

					// Starts the values at the current location with the previous set of values
					if (m_sync.back().first < position)
					{
						static SyncValues prev;
						prev = m_sync.back().second;
						m_sync.push_back({ position, prev });
					}

					if (strncmp(traversal.getCurrent(), "TS", 2) == 0)
					{
						traversal.move(2);
						uint32_t numerator = traversal.extractU32(), denom = 2;
							
						// Denom is optional, so use the no throw version
						traversal.extract(denom);
						m_sync.back().second.setTimeSig(numerator, denom);
					}
					else
					{
						switch (traversal.extractChar())
						{
						case 'b':
						case 'B':
							m_sync.back().second.setBPM(traversal.extractU32() * .001f);
							break;
						case 'a':
						case 'A':
							m_sync.back().second.setAnchor(traversal.extractU32());
						}
					}
				}
				catch (std::runtime_error err)
				{
					if (position != UINT32_MAX)
						std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << err.what() << std::endl;
					else
						std::cout << "Line " << traversal.getLineNumber() << ": position could not be parsed" << std::endl;
				}
				catch (const std::string& str)
				{
					std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << str << std::endl;
				}
				
				traversal.next();
			}
		}
		else if (m_version_cht > 1)
		{
			if (traversal.isTrackName("[Events]"))
			{
				traversal.resetPosition();
				while (traversal && traversal != '}' && traversal != '[')
				{
					uint32_t position = UINT32_MAX;
					try
					{
						position = traversal.extractPosition();

						if (strncmp(traversal.getCurrent(), "SE", 2) == 0)
						{
							traversal.move(2);
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
								m_sectionMarkers.emplace_back(position, traversal.extractText());
						}
						else if (traversal.extractChar() == 'E')
						{
							std::u32string str = traversal.extractText();
							if (str.compare(0, 7, U"section") == 0)
							{
								if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
									m_sectionMarkers.emplace_back(position, std::move(str.erase(0, 8)));
							}
							else
							{
								if (m_globalEvents.empty() || m_globalEvents.back().first < position)
								{
									static std::pair<uint32_t, std::vector<std::u32string>> pairNode;
									pairNode.first = position;
									m_globalEvents.push_back(pairNode);
								}

								m_globalEvents.back().second.emplace_back(std::move(str));
							}
						}
					}
					catch (std::runtime_error err)
					{
						if (position != UINT32_MAX)
							std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << err.what() << std::endl;
						else
							std::cout << "Line " << traversal.getLineNumber() << ": position could not be parsed" << std::endl;
					}
					catch (const std::string& str)
					{
						std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << str << std::endl;
					}

					traversal.next();
				}
			}
			else
			{
				int i = 0;
				while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
					++i;

				if (i < 11)
					s_noteTracks[i]->load_cht(traversal);
				else if (traversal != '[')
					traversal.skipTrack();
			}
		}
		else if (traversal.isTrackName("[Events]"))
		{
			traversal.resetPosition();

			// If reading version 1.X of the .chart format, construct the vocal track from this list
			uint32_t phrase = UINT32_MAX;
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position = UINT32_MAX;
				try
				{
					position = traversal.extractPosition();

					if (traversal.extractChar() == 'E')
					{
						std::u32string str = traversal.extractText();
						if (str.compare(0, 7, U"section") == 0)
						{
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
								m_sectionMarkers.push_back({ position, std::move(str.erase(0, 8))});
						}
						else if (str.compare(0, 5, U"lyric") == 0)
							reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9].get())->addLyric(0, position, std::move(str.erase(0, 6)));
						else if (str.compare(0, 12, U"phrase_start") == 0)
						{
							if (phrase < UINT32_MAX)
								reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9].get())->addPhrase(phrase, new LyricLine(position - phrase));
							phrase = position;
						}
						else if (str.compare(0, 10, U"phrase_end") == 0)
						{
							reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9].get())->addPhrase(phrase, new LyricLine(position - phrase));
							phrase = UINT32_MAX;
						}
						else
						{
							if (m_globalEvents.empty() || m_globalEvents.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::u32string>> pairNode;
								pairNode.first = position;
								m_globalEvents.push_back(pairNode);
							}

							m_globalEvents.back().second.push_back(std::move(str));
						}
					}
				}
				catch (std::runtime_error err)
				{
					if (position != UINT32_MAX)
						std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << err.what() << std::endl;
					else
						std::cout << "Line " << traversal.getLineNumber() << ": position could not be parsed" << std::endl;
				}
				catch (const std::string& str)
				{
					std::cout << "Line " << traversal.getLineNumber() << " - Position: " << position << str << std::endl;
				}

				traversal.next();
			}
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
				if (!m_ini.m_five_lane_drums.isActive() && !drumsLegacy.isFiveLane())
					ins = Instrument::Drums_Legacy;
				else if (m_ini.m_five_lane_drums || drumsLegacy.isFiveLane())
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
				switch (ins)
				{
				case Instrument::Guitar_lead:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_lead_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass_6:
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_rhythm:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_coop:
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Keys:
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_Legacy:
					drumsLegacy.load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get())->load_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_5:
					reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get())->load_chart_V1(difficulty, traversal);
					break;
				}
			}
			else if (traversal != '[')
				traversal.skipTrack();
		}
	}

	if (drumsLegacy.occupied())
	{
		if (!drumsLegacy.isFiveLane())
			*reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get()) = drumsLegacy;
		else
			*reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get()) = drumsLegacy;
	}
}

void Song::saveFile_Cht() const
{
	std::fstream outFile = FilestreamCheck::getFileStream(m_fullPath, std::ios_base::out | std::ios_base::trunc);
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
			outFile << '\t' << eventIter->first << " = E \"" << UnicodeString::U32ToStr(str) << "\"\n";
	}

	while (sectIter != m_sectionMarkers.end())
	{
		outFile << '\t' << sectIter->first << " = SE \"" << sectIter->second << "\"\n";
		++sectIter;
	}
	outFile << "}\n";

	for (const auto& track : s_noteTracks)
		track->save_cht(outFile);
	outFile.close();
}
