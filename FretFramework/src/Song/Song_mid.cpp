#include "Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

// Returns whether the event to be read is a MetaEvent.
// If not, the bufferPtr will already be moved past this event.
bool checkForMetaEvent(const unsigned char*& bufferPtr)
{
	static unsigned char syntax = 0xFF;
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
}

void Song::loadFile_Midi()
{
	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in | std::ios_base::binary);
	MidiChunk_Header header(inFile);
	m_tickrate = header.m_tickRate;
	m_hopo_frequency.setDefault(m_tickrate / 3);
	Sustainable::setForceThreshold(m_hopo_frequency);
	m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
	Sustainable::setsustainThreshold(m_sustain_cutoff_threshold);

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
			if (i != 0 && type == 3)
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
								if (type < 16)
								{
									int val = 0;
									if (strncmp((char*)current, "[section", 8) == 0)
										val = 1;
									else if (strncmp((char*)current, "[prc_", 5) == 0)
										val = 2;

									if (val > 0)
									{
										if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < position)
										{
											static std::pair<uint32_t, std::string> pairNode;
											pairNode.first = position;
											m_sectionMarkers.push_back(pairNode);
										}

										if (val == 1)
											m_sectionMarkers.back().second = std::string((char*)current + 9, length - 10);
										else
											m_sectionMarkers.back().second = std::string((char*)current + 5, length - 6);
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
				else if (name == "PART VOCALS")
					m_vocals.load_midi(0, current, end);
				else if (name == "HARM1")
					m_harmonies.load_midi(0, current, end);
				else if (name == "HARM2")
					m_harmonies.load_midi(1, current, end);
				else if (name == "HARM3")
					m_harmonies.load_midi(2, current, end);
			}
			else if (i == 0)
			{
				if (type == 3)
				{
					VariableLengthQuantity length(current);
					m_songInfo.name = std::string((char*)current, length);
					current += length;
					position += VariableLengthQuantity(current);
					if (!checkForMetaEvent(current))
						goto DeleteTrack;
					type = *current++;
				}

				do
				{
					if (type == 0x2F)
						break;

					VariableLengthQuantity length(current);

					switch (type)
					{
					case 0x51:
					case 0x58:
						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < position)
						{
							static SyncValues prev;
							prev = m_sync.back().second;
							m_sync.push_back({ position, prev });
						}

						if (type == 0x51)
						{
							uint32_t microsecondsPerQuarter = 0;
							memcpy((char*)&microsecondsPerQuarter + 1, current, length);
							m_sync.back().second.setBPM(60000000.0f / _byteswap_ulong(microsecondsPerQuarter));
						}
						else
							m_sync.back().second.setTimeSig(current[0], current[1]);
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

	DeleteTrack:
		delete[chunk.getLength()] track;
	}
	inFile.close();
}

void Song::saveFile_Midi(const std::filesystem::path& filepath) const
{
	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	MidiChunk_Header header(m_tickrate.m_value);
	header.writeToFile(outFile);

	MidiChunk_Track sync;
	if (m_songInfo.name.m_value.size())
		sync.addEvent(0, new MidiChunk_Track::MetaEvent_Text(3, m_songInfo.name.m_value));

	for (const auto& values : m_sync)
	{
		auto timeSig = values.second.getTimeSig();
		if (timeSig.first)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_TimeSignature(timeSig.first, timeSig.second, 24));

		float bpm = values.second.getBPM();
		if (bpm > 0)
			sync.addEvent(values.first, new MidiChunk_Track::MetaEvent_Tempo((uint32_t)roundf(60000000.0f / bpm)));
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
	header.m_numTracks += m_vocals.save_midi(outFile);
	header.m_numTracks += m_harmonies.save_midi(outFile);

	outFile.seekp(0);
	header.writeToFile(outFile);
	outFile.close();
}
