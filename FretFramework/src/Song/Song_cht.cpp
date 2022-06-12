#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include "Tracks\InstrumentalTracks\DrumTrack\DrumTrackConverter.h"
#include <iostream>

void Song::scanFile_Cht()
{
	m_version_cht = 1;
	// Loads the file into a char array and traverses it byte by byte
	// or by skipping to a new line character
	TextTraversal traversal(m_filepath);
	DrumNote_Legacy::resetLaning();
	InstrumentalTrack<DrumNote_Legacy> drumsLegacy("null", -1);
	int drumsLegacy_scan = 0;
	do
	{
		if (traversal == '}')
			traversal.next();

		if (traversal != '[')
			continue;

		traversal.setTrackName();
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
		}
		else if (traversal.isTrackName("[SyncTrack]") || traversal.isTrackName("[Events]"))
		{
			while (traversal && traversal != '}' && traversal != '[')
				traversal.next();
		}
		else if (m_version_cht > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
				++i;

			if (i < 11)
				m_noteTrackScans[i] |= s_noteTracks[i]->scan_cht(traversal);
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
				if (!m_ini.m_five_lane_drums.isActive() && !DrumNote_Legacy::isFiveLane())
					ins = Instrument::Drums_Legacy;
				else if (m_ini.m_five_lane_drums || DrumNote_Legacy::isFiveLane())
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
					m_noteTrackScans[0] |= reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_lead_6:
					m_noteTrackScans[1] |= reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass:
					m_noteTrackScans[2] |= reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_bass_6:
					m_noteTrackScans[3] |= reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_rhythm:
					m_noteTrackScans[4] |= reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Guitar_coop:
					m_noteTrackScans[5] |= reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Keys:
					m_noteTrackScans[6] |= reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_Legacy:
					drumsLegacy_scan |= drumsLegacy.scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_4:
					m_noteTrackScans[7] |= reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->scan_chart_V1(difficulty, traversal);
					break;
				case Instrument::Drums_5:
					m_noteTrackScans[8] |= reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->scan_chart_V1(difficulty, traversal);
					break;
				}
			}
			else
				traversal.skipTrack();
		}
	} while (traversal.next());

	if (drumsLegacy_scan)
		m_noteTrackScans[7 + DrumNote_Legacy::isFiveLane()] |= drumsLegacy_scan;
}

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

		traversal.setTrackName();
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
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position = UINT32_MAX;
				try
				{
					position = traversal.extractU32();

					// Ensures ascending order
					if (m_sync.back().first > position)
						throw "position out of order (previous:  " + std::to_string(m_sync.back().first) + ')';

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
		else if (traversal.isTrackName("[Events]"))
		{
			// If reading version 1.X of the .chart format, construct the vocal track from this list
			uint32_t phrase = UINT32_MAX;
			uint32_t prevPosition = 0;
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position = UINT32_MAX;
				try
				{
					position = traversal.extractU32();

					if (prevPosition > position)
						throw "position out of order (previous:  " + std::to_string(prevPosition) + ')';

					prevPosition = position;
					// Skip '='
					traversal.skipEqualsSign();

					if (strncmp(traversal.getCurrent(), "SE", 2) == 0)
					{
						traversal.move(2);
						if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
							m_sectionMarkers.push_back({ position, traversal.extractText() });
					}
					else if (traversal.extractChar() == 'E')
					{
						std::string ev = traversal.extractText();
						if (strncmp(ev.data(), "section", 7) == 0)
						{
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
								m_sectionMarkers.push_back({ position, ev.substr(8) });
							goto NextLine;
						}
						else if (m_version_cht < 2)
						{
							VocalTrack<1>* vocals = reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9]);
							if (strncmp(ev.data(), "lyric", 5) == 0)
								vocals->addLyric(0, position, ev.substr(6));
							else if (strncmp(ev.data(), "phrase_start", 12) == 0)
							{
								if (phrase < UINT32_MAX)
									vocals->addPhrase(phrase, new LyricLine(position - phrase));
								phrase = position;
							}
							else if (strncmp(ev.data(), "phrase_end", 10) == 0)
							{
								vocals->addPhrase(phrase, new LyricLine(position - phrase));
								phrase = UINT32_MAX;
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

						m_globalEvents.back().second.push_back(ev);
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

			NextLine:
				traversal.next();
			}
		}
		else if (m_version_cht > 1)
		{
			int i = 0;
			while (i < 11 && !traversal.isTrackName(s_noteTracks[i]->m_name))
				++i;

			if (i < 11)
				s_noteTracks[i]->load_cht(traversal);
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
				if (!m_ini.m_five_lane_drums.isActive() && !DrumNote_Legacy::isFiveLane())
					ins = Instrument::Drums_Legacy;
				else if (m_ini.m_five_lane_drums || DrumNote_Legacy::isFiveLane())
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
