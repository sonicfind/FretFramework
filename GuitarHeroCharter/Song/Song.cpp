#include "Song.h"
#include "Midi/MidiTrackWriter.h"
#include "..\FilestreamCheck.h"
#include <iostream>
using namespace MidiFile;

Song::Song(const std::filesystem::path& filepath)
	: m_filepath(filepath)
{
	m_version = 1;
	if (m_filepath.extension() == ".chart")
		loadFile_Chart();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
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
				outPath.replace_extension(".chart2.test");
				saveFile_Chart(outPath);
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

void Song::loadFile_Chart()
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
				WritableModifier<uint16_t> tickRate("Resolution");
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					std::string str;
					ss >> str;
					ss.ignore(5, '=');

					// Utilize short circuiting to stop if a read was valid
					m_version.read(str, ss) ||
						m_songInfo.name.read(str, ss) ||
						m_songInfo.artist.read(str, ss) ||
						m_songInfo.charter.read(str, ss) ||
						m_songInfo.album.read(str, ss) ||
						m_songInfo.year.read(str, ss) ||

						m_offset.read(str, ss) ||
						tickRate.read(str, ss) ||

						m_songInfo.difficulty.read(str, ss) ||
						m_songInfo.preview_start_time.read(str, ss) ||
						m_songInfo.preview_end_time.read(str, ss) ||
						m_songInfo.genre.read(str, ss) ||

						m_audioStreams.music.read(str, ss) ||
						m_audioStreams.guitar.read(str, ss) ||
						m_audioStreams.bass.read(str, ss) ||
						m_audioStreams.rhythm.read(str, ss) ||
						m_audioStreams.keys.read(str, ss) ||
						m_audioStreams.drum.read(str, ss) ||
						m_audioStreams.drum_2.read(str, ss) ||
						m_audioStreams.drum_3.read(str, ss) ||
						m_audioStreams.drum_4.read(str, ss) ||
						m_audioStreams.vocals.read(str, ss) ||
						m_audioStreams.crowd.read(str, ss);
				}
				Hittable::setTickRate(tickRate);
			}
			else if (line.find("SyncTrack") != std::string::npos)
			{
				// Default the first sync node to always mark
				SyncValues prev(true, true);
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					uint32_t position;
					ss >> position;
					ss.ignore(5, '=');
					// Starts the values at the current location with the previous set of values
					// Only if there is no node already here
					m_sync.insert({ position, prev });

					std::string type;
					ss >> type;
					if (type[0] == 'T')
					{
						uint32_t numerator, denom;
						ss >> numerator >> denom;
						if (!ss)
							denom = 2;
						m_sync.at(position).setTimeSig(numerator, denom);
					}
					else if (type[0] == 'B')
					{
						float bpm;
						ss >> bpm;
						m_sync.at(position).setBPM(bpm * .001f);
					}
					prev = m_sync.at(position);
					prev.unmarkBPM();
					prev.unmarkTimeSig();
				}
			}
			else if (line.find("Events") != std::string::npos)
			{
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					uint32_t position;
					ss >> position;
					ss.ignore(10, 'E');

					std::string ev;
					std::getline(ss, ev);
					// Substr calls to remove leading spaces and "" characters
					if (ev.find("section") != std::string::npos)
						m_sectionMarkers[position] = ev.substr(10, ev.length() - 11);
					else
						m_globalEvents[position].push_back(ev.substr(2, ev.length() - 3));
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

				int difficulty = 0;
				if (line.find("Expert") != std::string::npos)
					difficulty = 0;
				else if (line.find("Hard") != std::string::npos)
					difficulty = 1;
				else if (line.find("Medium") != std::string::npos)
					difficulty = 2;
				else if (line.find("Easy") != std::string::npos)
					difficulty = 3;

				switch (ins)
				{
				case Instrument::Guitar_lead:
					m_leadGuitar[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Guitar_lead_6:
					m_leadGuitar_6[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Guitar_bass:
					m_bassGuitar[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Guitar_bass_6:
					m_bassGuitar_6[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Guitar_rhythm:
					m_rhythmGuitar[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Guitar_coop:
					m_coopGuitar[difficulty].load_chart(inFile, m_version > 1);
					break;
				case Instrument::Drums:
					m_drums[difficulty].load_chart(inFile, m_version > 1);
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
	for (int i = 0; i < header.m_numTracks; ++i)
	{
		MidiChunk chunk(inFile);
		auto end = inFile.tellg() + std::streamoff(chunk.getLength());

		uint32_t position = VariableLengthQuantity(inFile);
		auto ev = MidiChunk_Track::parseEvent(syntax, inFile);
		if (ev->m_syntax != 0xFF)
			inFile.seekg(end);
		else
		{
			MidiChunk_Track::MetaEvent* meta = static_cast<MidiChunk_Track::MetaEvent*>(ev);
			if (meta->m_type == 3)
			{
				MidiChunk_Track::MetaEvent_Text* text = static_cast<MidiChunk_Track::MetaEvent_Text*>(meta);
				if (text->m_text == "EVENTS")
				{
					while (inFile.tellg() < end)
					{
						delete ev;
						position += VariableLengthQuantity(inFile);
						ev = MidiChunk_Track::parseEvent(syntax, inFile);
						if (ev->m_syntax == 0xFF)
						{
							meta = static_cast<MidiChunk_Track::MetaEvent*>(ev);
							if (meta->m_type == 0x01)
							{
								text = static_cast<MidiChunk_Track::MetaEvent_Text*>(meta);
								if (text->m_text.find("section") != std::string_view::npos)
								{
									size_t pos = text->m_text.find(' ') + 1;
									m_sectionMarkers[position] = text->m_text.substr(pos, text->m_text.length() - pos - 1);
								}
								else
									m_globalEvents[position].emplace_back(text->m_text);
							}
							else if (meta->m_type == 0x2F && inFile.tellg() != end)
								inFile.seekg(end);
						}
					}
				}
				else if (text->m_text == "PART GUITAR")
					m_leadGuitar.load_midi(inFile, end);
				else if (text->m_text == "PART GUITAR GHL")
					m_leadGuitar_6.load_midi(inFile, end);
				else if (text->m_text == "PART BASS")
					m_bassGuitar.load_midi(inFile, end);
				else if (text->m_text == "PART BASS GHL")
					m_bassGuitar_6.load_midi(inFile, end);
				else if (text->m_text == "PART GUITAR COOP")
					m_coopGuitar.load_midi(inFile, end);
				else if (text->m_text == "PART RHYTHM")
					m_rhythmGuitar.load_midi(inFile, end);
				else if (text->m_text == "PART DRUMS")
					m_drums.load_midi(inFile, end);
				else
					inFile.seekg(end);
			}
			else
			{
				// Read values into the tempomap
				SyncValues prev(true, true);

				// Set starting sync for safety
				m_sync.insert({ 0, prev });

				do
				{
					switch (meta->m_type)
					{
					case 0x51:
					case 0x58:
						// Starts the values at the current location with the previous set of values if it doesn't exist
						m_sync.insert({ position, prev });

						if (meta->m_type == 0x51)
							m_sync.at(position).setBPM(60000000.0f / static_cast<MidiChunk_Track::MetaEvent_Tempo*>(meta)->m_microsecondsPerQuarter);
						else
						{
							MidiChunk_Track::MetaEvent_TimeSignature* timeSig = static_cast<MidiChunk_Track::MetaEvent_TimeSignature*>(meta);
							m_sync.at(position).setTimeSig(timeSig->m_numerator, timeSig->m_denominator);
						}
						prev = m_sync.at(position);
						break;
					case 0x2F:
						inFile.seekg(end);
					}
					meta = nullptr;

					while (inFile.tellg() < end)
					{
						delete ev;
						position += VariableLengthQuantity(inFile);
						ev = MidiChunk_Track::parseEvent(syntax, inFile);
						if (ev->m_syntax == 0xFF)
						{
							meta = static_cast<MidiChunk_Track::MetaEvent*>(ev);
							break;
						}
					}
				} while (meta != nullptr);
			}
		}
		delete ev;
	}
	inFile.close();
}

void Song::saveFile_Chart(const std::filesystem::path& filepath) const
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
			outFile << "  " << sectIter->first << " = E \"section " << sectIter->second << "\"\n";
			++sectIter;
		}

		for (const auto& str : eventIter->second)
			outFile << "  " << eventIter->first << " = E \"" << str << "\"\n";
	}

	while (sectIter != m_sectionMarkers.end())
	{
		outFile << "  " << sectIter->first << " = E \"section " << sectIter->second << "\"\n";
		++sectIter;
	}
	outFile << "}\n";

	m_leadGuitar.save_chart("Single", outFile);
	m_leadGuitar_6.save_chart("GHLGuitar", outFile);
	m_bassGuitar.save_chart("DoubleBass", outFile);
	m_bassGuitar_6.save_chart("GHLBass", outFile);
	m_rhythmGuitar.save_chart("DoubleRhythm", outFile);
	m_coopGuitar.save_chart("DoubleGuitar", outFile);
	m_drums.save_chart("Drums", outFile);
	outFile.close();
}

void Song::saveFile_Midi(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	MidiChunk_Header header((uint16_t)Hittable::getTickRate());

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
	++header.m_numTracks;

	std::list<MidiTrackWriter*> instruments;
	if (m_leadGuitar.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<5>>("PART GUITAR", m_leadGuitar));
	if (m_leadGuitar_6.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<6>>("PART GUITAR GHL", m_leadGuitar_6));
	if (m_bassGuitar.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<5>>("PART BASS", m_bassGuitar));
	if (m_bassGuitar_6.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<6>>("PART BASS GHL", m_bassGuitar_6));
	if (m_coopGuitar.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<5>>("PART GUITAR COOP", m_coopGuitar));
	if (m_rhythmGuitar.occupied())
		instruments.push_back(new MidiTrackFiller<GuitarNote<5>>("PART RHYTHM", m_rhythmGuitar));
	if (m_drums.occupied())
		instruments.push_back(new MidiTrackFiller<DrumNote>("PART DRUMS", m_drums));
	header.m_numTracks += (uint16_t)instruments.size();

	header.writeToFile(outFile);
	sync.writeToFile(outFile);
	events.writeToFile(outFile);
	for (auto& track : instruments)
	{
		track->writeToFile(outFile);
		outFile.flush();
		delete track;
	}
	outFile.close();
}
