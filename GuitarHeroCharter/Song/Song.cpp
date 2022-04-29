#include "Song.h"
#include "..\FilestreamCheck.h"
#include <iostream>
using namespace MidiFile;

Song::Song(const std::filesystem::path& filepath)
	: m_filepath(filepath)
	, m_sync({ {0, SyncValues(true, true)} })
{
	if (m_filepath.extension() == ".chart" || m_filepath.extension() == ".cht")
		loadFile_Cht();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
	else
		throw InvalidExtensionException(m_filepath.extension().string());
	m_version = 2;
}

void Song::save() const
{
	try
	{
		std::filesystem::path outPath = m_filepath;
		while (true)
		{
			char answer = -1;
			std::cout << "Chart Type: ";
			std::cin >> answer;
			std::cin.clear();
			switch (answer)
			{
			case 'c':
			case 'C':
				outPath.replace_extension(".cht");
				saveFile_Cht(outPath);
				return;
			case 'm':
			case 'M':
				outPath.replace_extension(".mid.test");
				saveFile_Midi(outPath);
				return;
			case 'n':
			case 'N':
				return;
			}
		}
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}

std::filesystem::path Song::getFilepath()
{
	return m_filepath;
}

void Song::setFilepath(const std::filesystem::path& filename)
{
	m_filepath = filename;
}

void Song::loadFile_Cht()
{
	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in);
	std::string line;
	while (std::getline(inFile, line))
	{
		// Ensures that we're entering a scope
		if (line.find('[') != std::string::npos)
		{
			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (line.find("Song") != std::string::npos)
			{
				WritableModifier<uint16_t> tickRate("Resolution", 192);
				WritableModifier<std::string> oldYear("Year");
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
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
						tickRate.read(name, ss) ||

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

				Hittable::setTickRate(tickRate);
				if (m_version < 2 && !oldYear.m_value.empty())
				{
					char* end;
					m_songInfo.year = strtol(oldYear.m_value.substr(2).c_str(), &end, 0);
				}
			}
			else if (line.find("SyncTrack") != std::string::npos)
			{
				uint32_t prevPosition = 0;
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					uint32_t position;
					ss >> position;
					
					// Ensures ascending order
					if (prevPosition <= position)
					{
						prevPosition = position;
						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < position)
						{
							static SyncValues prev;
							prev = m_sync.back().second;
							m_sync.push_back({ position, prev });
						}
						ss.ignore(5, '=');

						std::string type;
						ss >> type;
						if (type[0] == 'T')
						{
							uint32_t numerator, denom;
							ss >> numerator >> denom;
							if (!ss)
								denom = 2;
							m_sync.back().second.setTimeSig(numerator, denom);
						}
						else if (type[0] == 'B')
						{
							float bpm;
							ss >> bpm;
							m_sync.back().second.setBPM(bpm * .001f);
						}
					}
				}
			}
			else if (line.find("Events") != std::string::npos)
			{
				uint32_t prevPosition = 0;
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					uint32_t position;
					ss >> position;

					// Ensures ascending order
					if (prevPosition <= position)
					{
						ss.ignore(10, 'E');

						std::string ev;
						std::getline(ss, ev);
						// Substr calls to remove leading spaces and "" characters
						if (ev.find("section") != std::string::npos)
						{
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
							{
								static std::pair<uint32_t, std::string> pairNode;
								pairNode.first = position;
								m_sectionMarkers.push_back(pairNode);
							}

							m_sectionMarkers.back().second = ev.substr(10, ev.length() - 11);
						}
						else
						{
							if (m_globalEvents.empty() || m_globalEvents.back().first < position)
							{
								static std::pair<uint32_t, std::vector<std::string>> pairNode;
								pairNode.first = position;
								m_globalEvents.push_back(pairNode);
							}

							m_globalEvents.back().second.push_back(ev.substr(2, ev.length() - 3));
						}
						prevPosition = position;
					}
				}
			}
			else if (m_version > 1)
			{
				if (line.find("Guitar") != std::string::npos)
					m_leadGuitar.load_cht(inFile);
				else if (line.find("Co-Op") != std::string::npos)
					m_coopGuitar.load_cht(inFile);
				else if (line.find("Bass") != std::string::npos)
					m_bassGuitar.load_cht(inFile);
				else if (line.find("Rhythm") != std::string::npos)
					m_rhythmGuitar.load_cht(inFile);
				else if (line.find("Drums") != std::string::npos)
					m_drums.load_cht(inFile);
				else if (line.find("Guitar_GHL") != std::string::npos)
					m_leadGuitar_6.load_cht(inFile);
				else if (line.find("Bass_GHL") != std::string::npos)
					m_bassGuitar_6.load_cht(inFile);
				else
				{
					inFile.ignore(std::numeric_limits<std::streamsize>::max(), '}');
					inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				}
			}
			else
			{
				Instrument ins = Instrument::None;
				if (line.find("Single") != std::string::npos)
					ins = Instrument::Guitar_lead;
				else if (line.find("DoubleGuitar") != std::string::npos)
					ins = Instrument::Guitar_coop;
				else if (line.find("DoubleBass") != std::string::npos)
					ins = Instrument::Guitar_bass;
				else if (line.find("DoubleRhythm") != std::string::npos)
					ins = Instrument::Guitar_rhythm;
				else if (line.find("Drums") != std::string::npos)
					ins = Instrument::Drums;
				else if (line.find("GHLGuitar") != std::string::npos)
					ins = Instrument::Guitar_lead_6;
				else if (line.find("GHLBass") != std::string::npos)
					ins = Instrument::Guitar_bass_6;
				else
				{
					inFile.ignore(std::numeric_limits<std::streamsize>::max(), '}');
					inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
					continue;
				}

				int difficulty = 3;
				if (line.find("Expert") != std::string::npos)
					difficulty = 3;
				else if (line.find("Hard") != std::string::npos)
					difficulty = 2;
				else if (line.find("Medium") != std::string::npos)
					difficulty = 1;
				else if (line.find("Easy") != std::string::npos)
					difficulty = 0;

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
		}
	}
	inFile.close();
}

void Song::loadFile_Midi()
{
	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in | std::ios_base::binary);
	MidiChunk_Header header(inFile);
	Hittable::setTickRate(header.m_tickRate);

	unsigned char syntax;
	// Returns whether the event to be read is a MetaEvent.
	// If not, the bufferPtr will already be moved past this event.
	auto checkForMetaEvent = [&](const unsigned char*& bufferPtr)
	{
		unsigned char tmpSyntax = *bufferPtr++;
		if (tmpSyntax & 0b10000000)
		{
			syntax = tmpSyntax;
			if (syntax == 0xFF)
				return true;

			switch (syntax)
			{
			case 0xF0:
			case 0xF7:
				bufferPtr += VariableLengthQuantity(bufferPtr);
				break;
			case 0x80:
			case 0x90:
			case 0xB0:
			case 0xA0:
			case 0xE0:
			case 0xF2:
				bufferPtr += 2;
				break;
			case 0xC0:
			case 0xD0:
			case 0xF3:
				++bufferPtr;
			}
		}
		else
		{
			switch (syntax)
			{
			case 0x80:
			case 0x90:
			case 0xB0:
			case 0xA0:
			case 0xE0:
			case 0xF2:
				++bufferPtr;
			}
		}
		return false;
	};

	for (int i = 0; i < header.m_numTracks; ++i)
	{
		MidiChunk chunk(inFile);
		const unsigned char* track = new unsigned char[chunk.getLength()];
		const unsigned char* const end = track + chunk.getLength();
		inFile.read((char*)track, chunk.getLength());
		
		const unsigned char* current = track;
		uint32_t position = VariableLengthQuantity(current);
		if (checkForMetaEvent(current))
		{
			unsigned char type = *current++;
			if (type == 3)
			{
				VariableLengthQuantity length(current);
				std::string name((char*)current, length);
				current += length;
				if (name == "EVENTS")
				{
					while (current < end)
					{
						position += VariableLengthQuantity(current);
						if (checkForMetaEvent(current))
						{
							type = *current++;
							if (type == 0x2F)
								break;
							else
							{
								length = VariableLengthQuantity(current);
								if (type == 0x01)
								{
									if (strncmp((char*)current, "[section", 8) == 0)
									{
										if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
										{
											static std::pair<uint32_t, std::string> pairNode;
											pairNode.first = position;
											m_sectionMarkers.push_back(pairNode);
										}

										m_sectionMarkers.back().second = std::string((char*)current + 9, length - 10);
									}
									else
									{
										if (m_globalEvents.empty() || m_globalEvents.back().first < position)
										{
											static std::pair<uint32_t, std::vector<std::string>> pairNode;
											pairNode.first = position;
											m_globalEvents.push_back(pairNode);
										}

										m_globalEvents.back().second.push_back(std::string((char*)current, length));
									}
								}
								current += length;
							}
						}
					}
				}
				else if (name == "PART GUITAR")
					m_leadGuitar.load_midi(current, end);
				else if (name == "PART GUITAR GHL")
					m_leadGuitar_6.load_midi(current, end);
				else if (name == "PART BASS")
					m_bassGuitar.load_midi(current, end);
				else if (name == "PART BASS GHL")
					m_bassGuitar_6.load_midi(current, end);
				else if (name == "PART GUITAR COOP")
					m_coopGuitar.load_midi(current, end);
				else if (name == "PART RHYTHM")
					m_rhythmGuitar.load_midi(current, end);
				else if (name == "PART DRUMS")
					m_drums.load_midi(current, end);
			}
			else
			{
				VariableLengthQuantity length(current);
				do
				{
					if (type == 0x2F)
						break;

					switch (type)
					{
					case 0x51:
					case 0x58:
						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < position)
							m_sync.push_back({ position, m_sync.back().second });

						if (type == 0x51)
						{
							uint32_t microsecondsPerQuarter = 0;
							memcpy((char*)&microsecondsPerQuarter + 1, current, length);
							m_sync.back().second.setBPM(60000000.0f / _byteswap_ulong(microsecondsPerQuarter));
						}
						else
						{
							static MidiChunk_Track::MetaEvent_TimeSignature timeSig(4, 2, 24);
							memcpy(&timeSig, current, length);
							m_sync.back().second.setTimeSig(timeSig.m_numerator, timeSig.m_denominator);
						}
						__fallthrough;
					default:
						current += length;
					}

					while (current < end)
					{
						position += VariableLengthQuantity(current);
						if (checkForMetaEvent(current))
						{
							type = *current++;
							break;
						}
					}
				} while (current < end);
			}
		}
		delete[chunk.getLength()] track;
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
	WritableModifier<uint16_t>("Resolution", Hittable::getTickRate()).write(outFile);

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
	outFile.close();
}

void Song::saveFile_Midi(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	MidiChunk_Header header((uint16_t)Hittable::getTickRate());
	header.writeToFile(outFile);

	MidiChunk_Track sync;
	for (const auto& values : m_sync)
	{
		float bpm = values.second.getBPM();
		if (bpm > 0)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_Tempo((uint32_t)roundf(60000000.0f / bpm)));

		auto timeSig = values.second.getTimeSig();
		if (timeSig.first)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_TimeSignature(timeSig.first, timeSig.second, 24));
	}
	sync.writeToFile(outFile);
	++header.m_numTracks;

	MidiChunk_Track events("EVENTS");
	auto sectIter = m_sectionMarkers.begin();
	for (auto eventIter = m_globalEvents.begin(); eventIter != m_globalEvents.end(); ++eventIter)
	{
		while (sectIter != m_sectionMarkers.end() && sectIter->first <= eventIter->first)
		{
			events.addEvent(sectIter->first, new MidiChunk_Track::MetaEvent_Text(1, "[section " + sectIter->second + ']'));
			++sectIter;
		}

		for (const auto& str : eventIter->second)
			events.addEvent(eventIter->first, new MidiChunk_Track::MetaEvent_Text(1, str));
	}

	while (sectIter != m_sectionMarkers.end())
	{
		events.addEvent(sectIter->first, new MidiChunk_Track::MetaEvent_Text(1, "[section " + sectIter->second + ']'));
		++sectIter;
	}
	events.writeToFile(outFile);
	++header.m_numTracks;

	if (m_leadGuitar.occupied())
	{
		m_leadGuitar.save_midi("PART GUITAR", outFile);
		++header.m_numTracks;
	}
	if (m_leadGuitar_6.occupied())
	{
		m_leadGuitar_6.save_midi("PART GUITAR GHL", outFile);
		++header.m_numTracks;
	}
	if (m_bassGuitar.occupied())
	{
		m_bassGuitar.save_midi("PART BASS", outFile);
		++header.m_numTracks;
	}
	if (m_bassGuitar_6.occupied())
	{
		m_bassGuitar_6.save_midi("PART BASS GHL", outFile);
		++header.m_numTracks;
	}
	if (m_coopGuitar.occupied())
	{
		m_coopGuitar.save_midi("PART GUITAR COOP", outFile);
		++header.m_numTracks;
	}
	if (m_rhythmGuitar.occupied())
	{
		m_rhythmGuitar.save_midi("PART RHYTHM", outFile);
		++header.m_numTracks;
	}
	if (m_drums.occupied())
	{
		m_drums.save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}

	outFile.seekp(0);
	header.writeToFile(outFile);
	outFile.close();
}
