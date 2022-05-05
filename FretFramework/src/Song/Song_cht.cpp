#include "Song.h"
#include "Tracks/BasicTracks/BasicTrack_cht.hpp"
#include "Tracks/VocalTracks/VocalTrack_cht.hpp"
#include "Chords/Chord_cht.hpp"
#include "..\FilestreamCheck.h"
#include <iostream>

void Song::loadFile_Cht()
{
	static char buffer[512] = { 0 };
	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in);
	while (inFile.getline(buffer, 512))
	{
		// Ensures that we're entering a scope
		if (strchr(buffer, '['))
		{
			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (strstr(buffer, "Song"))
			{
				WritableModifier<std::string> oldYear("Year");
				while (inFile.getline(buffer, 512) && buffer[0] != '}')
				{
					std::stringstream ss(buffer);
					std::string name;
					ss >> name;
					ss.ignore(5, '=');

					// Utilize short circuiting to stop if a read was valid
					m_version.read(name, ss) ||

						m_songInfo.name.read(name, ss) ||
						m_songInfo.artist.read(name, ss) ||
						m_songInfo.charter.read(name, ss) ||
						m_songInfo.album.read(name, ss) ||
						(m_version < 2 && oldYear.read(name, ss)) ||
						(m_version >= 2 && m_songInfo.year.read(name, ss)) ||

						m_offset.read(name, ss) ||
						m_tickrate.read(name, ss) ||
						m_hopo_frequency.read(name, ss) ||
						m_sustain_cutoff_threshold.read(name, ss) ||

						m_songInfo.difficulty.read(name, ss) ||
						m_songInfo.preview_start_time.read(name, ss) ||
						m_songInfo.preview_end_time.read(name, ss) ||
						m_songInfo.genre.read(name, ss) ||

						m_audioStreams.music.read(name, ss) ||
						m_audioStreams.guitar.read(name, ss) ||
						m_audioStreams.bass.read(name, ss) ||
						m_audioStreams.rhythm.read(name, ss) ||
						m_audioStreams.keys.read(name, ss) ||
						m_audioStreams.drum.read(name, ss) ||
						m_audioStreams.drum_2.read(name, ss) ||
						m_audioStreams.drum_3.read(name, ss) ||
						m_audioStreams.drum_4.read(name, ss) ||
						m_audioStreams.vocals.read(name, ss) ||
						m_audioStreams.crowd.read(name, ss);
				}

				// Sets the threshold for forcing guitar notes and for sustains
				// Automatically sets the threshold 1/3 of the tickrate if they are at the default value
				m_hopo_frequency.setDefault(m_tickrate / 3);
				Sustainable::setForceThreshold(m_hopo_frequency);
				m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
				Sustainable::setsustainThreshold(m_sustain_cutoff_threshold);
				if (m_version < 2 && !oldYear.m_value.empty())
				{
					char* end;
					m_songInfo.year = strtol(oldYear.m_value.substr(2).c_str(), &end, 0);
				}
			}
			else if (strstr(buffer, "SyncTrack"))
			{
				uint32_t prevPosition = 0;
				while (inFile.getline(buffer, 512) && buffer[0] != '}')
				{
					const char* str = buffer;
					int count;
					uint32_t position;
					sscanf_s(str, " %lu%n", &position, &count);

					// Ensures ascending order
					if (prevPosition <= position)
					{
						prevPosition = position;
						str += count;

						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < position)
						{
							static SyncValues prev;
							prev = m_sync.back().second;
							m_sync.push_back({ position, prev });
						}

						char type[3] = { 0 };
						sscanf_s(str, " = %[^ ]%n", &type, 3, &count);
						str += count;
						if (strcmp(type, "TS") == 0)
						{
							uint32_t numerator = 4, denom = 2;
							sscanf_s(str, " %lu %lu", &numerator, &denom);
							m_sync.back().second.setTimeSig(numerator, denom);
						}
						else if (strcmp(type, "B") == 0)
						{
							uint32_t bpm = 120000;
							sscanf_s(str, " %lu", &bpm);
							m_sync.back().second.setBPM(bpm * .001f);
						}
					}
				}
			}
			else if (strstr(buffer, "Events"))
			{
				// If reading version 1.X of the .chart format, construct the vocal track from this list
				uint32_t phrase = 0;
				bool phraseActive = false;
				uint32_t prevPosition = 0;
				while (inFile.getline(buffer, 512) && buffer[0] != '}')
				{
					char* str = buffer;
					uint32_t position;
					char type[3] = { 0 };
					int count;
					sscanf_s(str, " %lu = %[^ ]%n", &position, type, 3, &count);
					// Ensures ascending order
					if (prevPosition <= position)
					{
						prevPosition = position;
						str += count + 1;
						if (*str == '\"')
							++str;

						char strBuf[256] = { 0 };
						sscanf_s(str, "%[^\"]s", &strBuf, 256);
						if (strcmp(type, "E") == 0)
						{
							if (strstr(strBuf, "section"))
							{
								if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
									m_sectionMarkers.push_back({ position, strBuf + 8 });
							}
							else if (m_version < 2)
							{
								if (strstr(strBuf, "lyric"))
									m_vocals.addLyric(0, position, strBuf + 6);
								else if (strstr(strBuf, "phrase_start"))
								{
									if (phraseActive)
										m_vocals.addPhrase(phrase, new LyricLine(position - phrase));
									phrase = position;
									phraseActive = true;
								}
								else if (strstr(strBuf, "phrase_end"))
								{
									m_vocals.addPhrase(phrase, new LyricLine(position - phrase));
									phraseActive = false;
								}
								else
									goto WriteAsGlobalEvent;
								continue;
							}

						WriteAsGlobalEvent:
							if (m_globalEvents.empty() || m_globalEvents.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::string>> pairNode;
								pairNode.first = position;
								m_globalEvents.push_back(pairNode);
							}

							m_globalEvents.back().second.push_back(strBuf);
						}
						else if (strcmp(type, "SE") == 0)
						{
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
								m_sectionMarkers.push_back({ position, strBuf });
						}
					}
				}
			}
			else if (m_version > 1)
			{
				if (strcmp(buffer, "[LeadGuitar]") == 0)
					m_leadGuitar.load_cht(inFile);
				else if (strcmp(buffer, "[CoopGuitar]") == 0)
					m_coopGuitar.load_cht(inFile);
				else if (strcmp(buffer, "[BassGuitar]") == 0)
					m_bassGuitar.load_cht(inFile);
				else if (strcmp(buffer, "[RhythmGuitar]") == 0)
					m_rhythmGuitar.load_cht(inFile);
				else if (strcmp(buffer, "[Drums]") == 0)
					m_drums.load_cht(inFile);
				else if (strcmp(buffer, "[LeadGuitar_GHL]") == 0)
					m_leadGuitar_6.load_cht(inFile);
				else if (strcmp(buffer, "[BassGuitar_GHL]") == 0)
					m_bassGuitar_6.load_cht(inFile);
				else if (strcmp(buffer, "[Vocals]") == 0)
					m_vocals.load_cht(inFile);
				else if (strcmp(buffer, "[Harmonies]") == 0)
					m_harmonies.load_cht(inFile);
				else
					while (inFile.getline(buffer, 512) && buffer[0] != '}');
			}
			else
			{
				Instrument ins = Instrument::None;
				if (strstr(buffer, "Single"))
					ins = Instrument::Guitar_lead;
				else if (strstr(buffer, "DoubleGuitar"))
					ins = Instrument::Guitar_coop;
				else if (strstr(buffer, "DoubleBass"))
					ins = Instrument::Guitar_bass;
				else if (strstr(buffer, "DoubleRhythm"))
					ins = Instrument::Guitar_rhythm;
				else if (strstr(buffer, "Drums"))
					ins = Instrument::Drums;
				else if (strstr(buffer, "GHLGuitar"))
					ins = Instrument::Guitar_lead_6;
				else if (strstr(buffer, "GHLBass"))
					ins = Instrument::Guitar_bass_6;
				else
				{
					while (inFile.getline(buffer, 512) && buffer[0] != '}');
					continue;
				}

				int difficulty = 4;
				if (strstr(buffer, "Expert"))
					difficulty = 3;
				else if (strstr(buffer, "Hard"))
					difficulty = 2;
				else if (strstr(buffer, "Medium"))
					difficulty = 1;
				else if (strstr(buffer, "Easy"))
					difficulty = 0;

				try
				{
					switch (ins)
					{
					case Instrument::Guitar_lead:
						m_leadGuitar[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Guitar_lead_6:
						m_leadGuitar_6[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Guitar_bass:
						m_bassGuitar[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Guitar_bass_6:
						m_bassGuitar_6[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Guitar_rhythm:
						m_rhythmGuitar[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Guitar_coop:
						m_coopGuitar[difficulty].load_chart_V1(inFile);
						break;
					case Instrument::Drums:
						m_drums[difficulty].load_chart_V1(inFile);
						break;
					}
				}
				catch (EndofFileException)
				{
					std::string name;
					switch (difficulty)
					{
					case 0:
						name = "Easy";
						break;
					case 1:
						name = "Medium";
						break;
					case 2:
						name = "Hard";
						break;
					case 3:
						name = "Expert";
						break;
					case 4:
						name = "Unknown";
						break;
					}

					switch (ins)
					{
					case Instrument::Guitar_lead:
						name += "Single";
						break;
					case Instrument::Guitar_lead_6:
						name += "GHLGuitar";
						break;
					case Instrument::Guitar_bass:
						name += "DoubleBass";
						break;
					case Instrument::Guitar_bass_6:
						name += "GHLBass";
						break;
					case Instrument::Guitar_rhythm:
						name += "DoubleRhythm";
						break;
					case Instrument::Guitar_coop:
						name += "DoubleGuitar";
						break;
					case Instrument::Drums:
						name += "Drums";
						break;
					}
					std::cout << "Error in track " << name << std::endl;
					throw EndofFileException();
				}
			}
		}
	}
	inFile.close();
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
		sync.second.writeSync_chart(sync.first, outFile);
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
