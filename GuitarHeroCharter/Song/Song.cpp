#include "Song.h"
#include "Midi/FileHandler/MidiFile.h"
#include "..\FilestreamCheck.h"
#include <iostream>
using namespace MidiFile;

Song::Song(const std::filesystem::path& filepath)
	: m_filepath(filepath)
{
	if (m_filepath.extension() == ".chart")
		loadFile_Chart();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
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
				while (std::getline(inFile, line) && line.find('}') == std::string::npos)
				{
					std::stringstream ss(line);
					std::string str;
					ss >> str;
					ss.ignore(5, '=');

					// Utilize short circuiting to stop if a read was valid
					m_songInfo.name.read(str, ss) ||
						m_songInfo.artist.read(str, ss) ||
						m_songInfo.charter.read(str, ss) ||
						m_songInfo.album.read(str, ss) ||
						m_songInfo.year.read(str, ss) ||

						m_offset.read(str, ss) ||
						m_ticks_per_beat.read(str, ss) ||

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

				bool version2 = m_filepath.extension() == ".chart2";
				switch (ins)
				{
				case Instrument::Guitar_lead:
					m_leadGuitar.load_chart(line, inFile, version2);
					break;
				case Instrument::Guitar_lead_6:
					m_leadGuitar_6.load_chart(line, inFile, version2);
					break;
				case Instrument::Guitar_bass:
					m_bassGuitar.load_chart(line, inFile, version2);
					break;
				case Instrument::Guitar_bass_6:
					m_bassGuitar_6.load_chart(line, inFile, version2);
					break;
				case Instrument::Guitar_rhythm:
					m_rhythmGuitar.load_chart(line, inFile, version2);
					break;
				case Instrument::Guitar_coop:
					m_coopGuitar.load_chart(line, inFile, version2);
					break;
				case Instrument::Drums:
					m_drums.load_chart(line, inFile, version2);
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
	m_ticks_per_beat.set(header.m_tickRate);
	for (int i = 0; i < header.m_numTracks; ++i)
	{
		MidiFile::MidiChunk_Track track(inFile);
		if (track.getName().empty())
		{
			// Read values into the tempomap

			SyncValues prev(true, true);
			for (auto& vec : track)
			{
				// Starts the values at the current location with the previous set of values
				// Only if there is no node already here
				m_sync.insert({ vec.first, prev });
				for (auto ptr : vec.second)
				{
					if (ptr->m_syntax == 0xFF)
					{
						MidiChunk_Track::MetaEvent* ev = static_cast<MidiChunk_Track::MetaEvent*>(ptr);
						if (ev->m_type == 0x51)
						{
							MidiChunk_Track::MetaEvent_Tempo* tempo = static_cast<MidiChunk_Track::MetaEvent_Tempo*>(ptr);
							m_sync.at(vec.first).setBPM(60000000.0f / tempo->m_microsecondsPerQuarter);
						}
						else if (ev->m_type == 0x58)
						{
							MidiChunk_Track::MetaEvent_TimeSignature* timeSig = static_cast<MidiChunk_Track::MetaEvent_TimeSignature*>(ptr);
							m_sync.at(vec.first).setTimeSig(timeSig->m_numerator, timeSig->m_denominator);
						}
					}
				}
				prev = m_sync.at(vec.first);
				prev.unmarkBPM();
				prev.unmarkTimeSig();
			}
		}
		else if (track.getName() == "EVENTS")
		{
			for (auto& vec : track)
			{
				for (auto ptr : vec.second)
				{
					if (ptr->m_syntax == 0xFF)
					{
						MidiChunk_Track::MetaEvent* ev = static_cast<MidiChunk_Track::MetaEvent*>(ptr);
						if (ev->m_type == 0x01)
						{
							MidiChunk_Track::MetaEvent_Text* text = static_cast<MidiChunk_Track::MetaEvent_Text*>(ptr);
							if (text->m_text.find("section") != std::string_view::npos)
							{
								size_t pos = text->m_text.find(' ') + 1;
								m_sectionMarkers[vec.first] = text->m_text.substr(pos, text->m_text.length() - pos - 1);
							}
							else
								m_globalEvents[vec.first].emplace_back(text->m_text);
						}
					}
				}
			}
		}
		else if (track.getName() == "PART GUITAR")
			m_leadGuitar.load_midi(track);
		else if (track.getName() == "PART GUITAR GHL")
			m_leadGuitar_6.load_midi(track);
		else if (track.getName() == "PART BASS")
			m_bassGuitar.load_midi(track);
		else if (track.getName() == "PART BASS GHL")
			m_bassGuitar_6.load_midi(track);
		else if (track.getName() == "PART GUITAR COOP")
			m_coopGuitar.load_midi(track);
		else if (track.getName() == "PART RHYTHM")
			m_rhythmGuitar.load_midi(track);
		else if (track.getName() == "PART DRUMS")
			m_drums.load_midi(track);
		else if (track.getName() == "PART VOCALS")
		{}
		else if (track.getName() == "PART KEYS")
		{}
	}
	inFile.close();
}

void Song::saveFile_Chart(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc);
	outFile << "[Chart]\n{\n";
	m_songInfo.name.write(outFile);
	m_songInfo.artist.write(outFile);
	m_songInfo.charter.write(outFile);
	m_songInfo.album.write(outFile);
	m_songInfo.year.write(outFile);

	m_offset.write(outFile);
	m_ticks_per_beat.write(outFile);

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
}
