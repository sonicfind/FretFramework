#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile(TextTraversal&& traversal)
{
	int version = 0;
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
			static std::pair<std::string_view, ModifierNode> constexpr PREDEFINED_MODIFIERS[]
			{
				{ "Album",        { "album", ModifierNode::STRING_CHART } },
				{ "Artist",       { "artist", ModifierNode::STRING_CHART } },
				{ "BassStream",   { "BassStream", ModifierNode::STRING_CHART } },
				{ "Charter",      { "charter", ModifierNode::STRING_CHART } },
				{ "CrowdStream",  { "CrowdStream", ModifierNode::STRING_CHART } },
				{ "Difficulty",   { "diff_band", ModifierNode::INT32} },
				{ "Drum2Stream",  { "Drum2Stream", ModifierNode::STRING_CHART } },
				{ "Drum3Stream",  { "Drum3Stream", ModifierNode::STRING_CHART } },
				{ "Drum4Stream",  { "Drum4Stream", ModifierNode::STRING_CHART } },
				{ "DrumStream",   { "DrumStream", ModifierNode::STRING_CHART } },
				{ "FileVersion",  { "FileVersion", ModifierNode::UINT16} },
				{ "Genre",        { "genre", ModifierNode::STRING_CHART } },
				{ "GuitarStream", { "GuitarStream", ModifierNode::STRING_CHART } },
				{ "KeysStream",   { "KeysStream", ModifierNode::STRING_CHART } },
				{ "MusicStream",  { "MusicStream", ModifierNode::STRING_CHART } },
				{ "Name",         { "name", ModifierNode::STRING_CHART } },
				{ "Offset",       { "delay", ModifierNode::FLOAT} },
				{ "PreviewEnd",   { "preview_end_time", ModifierNode::FLOAT} },
				{ "PreviewStart", { "preview_start_time", ModifierNode::FLOAT} },
				{ "Resolution",   { "Resolution", ModifierNode::UINT16} },
				{ "RhythmStream", { "RhythmStream", ModifierNode::STRING_CHART } },
				{ "VocalStream",  { "VocalStream", ModifierNode::STRING_CHART } },
				{ "Year",         { "year", ModifierNode::STRING_CHART } },
			};

			bool versionChecked = false;
			bool resolutionChecked = false;
			while (traversal && traversal != '}' && traversal != '[')
			{
				if (auto node = traversal.testForModifierName(PREDEFINED_MODIFIERS))
				{
					if (node->name[0] == 'F')
					{
						if (!versionChecked)
						{
							version = traversal.extract<uint16_t>();;
							versionChecked = true;
						}
					}
					else if (node->name[0] == 'R' && node->name[1] == 'e')
					{
						if (!resolutionChecked)
						{
							m_tickrate = traversal.extract<uint16_t>();;
							resolutionChecked = true;
						}
					}
					else if (!getModifier(node->name))
						m_modifiers.push_back(traversal.createModifier(node));
				}
				traversal.next();
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
						uint32_t numerator = traversal.extract<uint32_t>(), denom = 2;
							
						// Denom is optional, so use the no throw version
						traversal.extract(denom);
						m_sync.back().second.setTimeSig(numerator, denom);
					}
					else
					{
						switch (traversal.extract<unsigned char>())
						{
						case 'B':
						case 'b':
							m_sync.back().second.setBPM(traversal.extract<uint32_t>() * .001f);
							break;
						case 'A':
						case 'a':
							m_sync.back().second.setAnchor(traversal.extract<uint32_t>());
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
		else if (version > 1)
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
						else if (traversal.extract<unsigned char>() == 'E')
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

					if (traversal.extract<unsigned char>() == 'E')
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
				if (BooleanModifier* fiveLaneDrums = getModifier<BooleanModifier>("five_lane_drums"))
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
	s_VERSION_CHT.write(outFile);
	m_tickrate.write(outFile);

	for (const auto& modifier : m_modifiers)
		if (modifier->getName()[0] <= 90)
			modifier->write(outFile);

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
