#include "Song.h"
#include "Tracks/BasicTracks/BasicTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "..\FilestreamCheck.h"
#include "..\TextFileTraversal.h"
#include <iostream>

void Song::loadFile_Cht()
{
	// Loads the file into a char array and traverses it byte by byte
	// or by skipping to a new line character
	TextTraversal traversal(m_filepath);
	while (traversal)
	{
		if (traversal != '[')
		{
			traversal.next();
			continue;
		}

		const char* const trackName = traversal.getCurrent();
		traversal.next();

		if (traversal == '{')
			traversal.next();

		if (strncmp(trackName, "[Song]", 6) == 0)
		{
			WritableModifier<std::string> oldYear("Year");
			while (traversal && traversal != '}' && traversal != '[')
			{
				// Utilize short circuiting to stop if a read was valid
				m_version.read(traversal) ||

					m_songInfo.name.read(traversal) ||
					m_songInfo.artist.read(traversal) ||
					m_songInfo.charter.read(traversal) ||
					m_songInfo.album.read(traversal) ||
					(m_version < 2 && oldYear.read(traversal)) ||
					(m_version >= 2 && m_songInfo.year.read(traversal)) ||

					m_offset.read(traversal) ||
					m_tickrate.read(traversal) ||
					m_hopo_frequency.read(traversal) ||
					m_sustain_cutoff_threshold.read(traversal) ||

					m_songInfo.difficulty.read(traversal) ||
					m_songInfo.preview_start_time.read(traversal) ||
					m_songInfo.preview_end_time.read(traversal) ||
					m_songInfo.genre.read(traversal) ||

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

			// Sets the threshold for forcing guitar notes and for sustains
			// Automatically sets the threshold 1/3 of the tickrate if they are at the default value
			m_hopo_frequency.setDefault(m_tickrate / 3);
			Sustainable::setForceThreshold(m_hopo_frequency);
			m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
			Sustainable::setsustainThreshold(m_sustain_cutoff_threshold);
			if (m_version < 2 && !oldYear.m_value.empty())
				m_songInfo.year = strtol(oldYear.m_value.substr(2).c_str(), nullptr, 0);
		}
		else if (strncmp(trackName, "[SyncTrack]", 11) == 0)
		{
			while (traversal && traversal != '}' && traversal != '[')
			{
				uint32_t position;
				if (traversal.extractUInt(position))
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
							if (traversal.extractUInt(numerator))
							{
								traversal.extractUInt(denom);
								m_sync.back().second.setTimeSig(numerator, denom);
							}
						}
						else
						{
							switch (traversal.extractChar())
							{
							case 'b':
							case 'B':
							{
								uint32_t bpm = 120000;
								if (traversal.extractUInt(bpm))
									m_sync.back().second.setBPM(bpm * .001f);
							}
								break;
							case 'a':
							case 'A':
							{
								uint32_t anchor = 0;
								if (traversal.extractUInt(anchor))
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
				if (traversal.extractUInt(position))
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
						else if (traversal.extractChar() == 'E')
						{
							std::string_view ev = traversal.extractText();
							if (strncmp(ev.data(), "section", 7) == 0)
							{
								if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
									m_sectionMarkers.push_back({ position, std::string(ev.substr(8)) });
								goto NextLine;
							}
							else if (m_version < 2)
							{
								if (strncmp(ev.data(), "lyric", 5) == 0)
									m_vocals.addLyric(0, position, std::string(ev.substr(6)));
								else if (strncmp(ev.data(), "phrase_start", 12) == 0)
								{
									if (phraseActive)
										m_vocals.addPhrase(phrase, new LyricLine(position - phrase));
									phrase = position;
									phraseActive = true;
								}
								else if (strncmp(ev.data(), "phrase_end", 10) == 0)
								{
									m_vocals.addPhrase(phrase, new LyricLine(position - phrase));
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
		else if (m_version > 1)
		{
			if (strncmp(trackName, "[LeadGuitar]", 12) == 0)
				m_leadGuitar.load_cht(traversal);
			else if (strncmp(trackName, "[CoopGuitar]", 12) == 0)
				m_coopGuitar.load_cht(traversal);
			else if (strncmp(trackName, "[BassGuitar]", 12) == 0)
				m_bassGuitar.load_cht(traversal);
			else if (strncmp(trackName, "[RhythmGuitar]", 14) == 0)
				m_rhythmGuitar.load_cht(traversal);
			else if (strncmp(trackName, "[Drums]", 7) == 0)
				m_drums.load_cht(traversal);
			else if (strncmp(trackName, "[LeadGuitar_GHL]", 16) == 0)
				m_leadGuitar_6.load_cht(traversal);
			else if (strncmp(trackName, "[BassGuitar_GHL]", 16) == 0)
				m_bassGuitar_6.load_cht(traversal);
			else if (strncmp(trackName, "[Vocals]", 8) == 0)
				m_vocals.load_cht(traversal);
			else if (strncmp(trackName, "[Harmonies]", 11) == 0)
				m_harmonies.load_cht(traversal);
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
				ins = Instrument::Drums;
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
					m_leadGuitar[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Guitar_lead_6:
					m_leadGuitar_6[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Guitar_bass:
					m_bassGuitar[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Guitar_bass_6:
					m_bassGuitar_6[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Guitar_rhythm:
					m_rhythmGuitar[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Guitar_coop:
					m_coopGuitar[difficulty].load_chart_V1(traversal);
					break;
				case Instrument::Drums:
					m_drums[difficulty].load_chart_V1(traversal);
					break;
				}
			}
			else
				traversal.skipTrack();
		}

		if (traversal == '}')
			traversal.next();
	}
}

void Song::saveFile_Cht(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc);
	outFile << "[Song]\n{\n";
	m_version.write(outFile);
	m_songInfo.name.write(outFile);
	m_songInfo.artist.write(outFile);
	m_songInfo.charter.write(outFile);
	m_songInfo.album.write(outFile);
	m_songInfo.year.write(outFile);

	m_offset.write(outFile);
	m_tickrate.write(outFile);
	m_hopo_frequency.write(outFile);
	m_sustain_cutoff_threshold.write(outFile);

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

	m_leadGuitar.save_cht(outFile);
	m_leadGuitar_6.save_cht(outFile);
	m_bassGuitar.save_cht(outFile);
	m_bassGuitar_6.save_cht(outFile);
	m_rhythmGuitar.save_cht(outFile);
	m_coopGuitar.save_cht(outFile);
	m_drums.save_cht(outFile);
	m_vocals.save_cht(outFile);
	m_harmonies.save_cht(outFile);
	outFile.close();
}
