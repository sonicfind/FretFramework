#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile(TextTraversal&& traversal)
{
	m_version_cht = 1;
	m_tickrate = 192;
	Sustainable::setForceThreshold(64);
	Sustainable::setsustainThreshold(64);
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
				traverseCHTSongSection(this, traversal, SongModMapping::s_FULLMODIFIERMAP);

				if (!m_songInfo.year.m_string->empty() && m_songInfo.year.m_string[0] == ',')
				{
					auto iter = m_songInfo.year.m_string->begin() + 1;
					while (iter != m_songInfo.year.m_string->end() && *iter == ' ')
						++iter;
					m_songInfo.year.m_string->erase(m_songInfo.year.m_string->begin(), iter);
				}

				m_ini.setBaseModifiers();

				m_ini.getName() = m_songInfo.name;
				m_ini.getArtist() = m_songInfo.artist;
				m_ini.getCharter() = m_songInfo.charter;
				m_ini.getAlbum() = m_songInfo.album;
				m_ini.getYear() = m_songInfo.year;
				m_ini.getGenre() = m_songInfo.genre;

				if (m_songInfo.preview_start_time)
					m_ini.setModifier<NumberModifier<float>>("preview_start_time", m_songInfo.preview_start_time);
				if (m_songInfo.preview_end_time)
					m_ini.setModifier<NumberModifier<float>>("preview_end_time", m_songInfo.preview_end_time);
				if (m_songInfo.difficulty)
					m_ini.setModifier<NumberModifier<int16_t>>("diff_band", m_songInfo.difficulty);
				if (m_offset)
					m_ini.setModifier<NumberModifier<float>>("delay", m_offset);
			}
			else
			{
				m_songInfo.name = m_ini.getName();
				m_songInfo.artist = m_ini.getArtist();
				m_songInfo.charter = m_ini.getCharter();
				m_songInfo.album = m_ini.getAlbum();
				m_songInfo.year = m_ini.getYear();
				m_songInfo.genre = m_ini.getGenre();

				if (auto* startTime = m_ini.getModifier<NumberModifier<float>>("preview_start_time"))
					m_songInfo.preview_start_time = *startTime;

				if (auto* endTime = m_ini.getModifier<NumberModifier<float>>("preview_end_time"))
					m_songInfo.preview_end_time = *endTime;

				if (auto* diffBand = m_ini.getModifier<NumberModifier<int16_t>>("diff_band"))
					m_songInfo.difficulty = *diffBand;

				traverseCHTSongSection(this, traversal, SongModMapping::s_AUDIOSTREAMMAP);

				if (auto* delay = m_ini.getModifier<NumberModifier<float>>("delay"))
					m_offset = *delay;
				else if (m_offset)
					m_ini.setModifier<NumberModifier<float>>("delay", m_offset);
			}

			Sustainable::setForceThreshold(m_tickrate / 3);
			Sustainable::setsustainThreshold(m_tickrate / 3);
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
						uint32_t numerator = traversal.extractInt<uint32_t>(), denom = 2;
							
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
							m_sync.back().second.setBPM(traversal.extractInt<uint32_t>() * .001f);
							break;
						case 'a':
						case 'A':
							m_sync.back().second.setAnchor(traversal.extractInt<uint32_t>());
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
				if (BooleanModifier* fiveLaneDrums = m_ini.getModifier<BooleanModifier>("five_lane_drums"))
				{
					if (fiveLaneDrums->m_boolean)
						ins = Instrument::Drums_5;
					else
						ins = Instrument::Drums_4;
				}
				else if (drumsLegacy.isFiveLane())
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
