#include "Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"
#include "FileChecks/FilestreamCheck.h"
#include "Tracks/InstrumentalTracks/DrumTrack/DrumTrackConverter.h"
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
	if (m_ini.m_star_power_note != 116)
		NoteTrack::s_starPowerReadNote = (unsigned char)m_ini.m_multiplier_note;
	else if (m_ini.m_multiplier_note != 116)
		NoteTrack::s_starPowerReadNote = (unsigned char)m_ini.m_star_power_note;
	else
		NoteTrack::s_starPowerReadNote = 116;

	std::fstream inFile = FilestreamCheck::getFileStream(m_filepath, std::ios_base::in | std::ios_base::binary);
	MidiChunk_Header header(inFile);
	m_tickrate = header.m_tickRate;

	if (m_ini.m_eighthnote_hopo)
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 2);
	else
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 3);

	m_ini.m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	DrumNote_Legacy::resetLaning();
	InstrumentalTrack<DrumNote_Legacy> drumsLegacy("null", -1);
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
				else if (name == "PART GUITAR" || name == "T1 GEMS")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->load_midi(current, end);
				else if (name == "PART GUITAR GHL")
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->load_midi(current, end);
				else if (name == "PART BASS")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->load_midi(current, end);
				else if (name == "PART BASS GHL")
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->load_midi(current, end);
				else if (name == "PART RHYTHM")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->load_midi(current, end);
				else if (name == "PART GUITAR COOP")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->load_midi(current, end);
				else if (name == "PART KEYS")
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->load_midi(current, end);
				else if (name == "PART DRUMS")
				{
					if (!m_ini.m_five_lane_drums.isActive())
						drumsLegacy.load_midi(current, end);
					else if (!m_ini.m_five_lane_drums)
						reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->load_midi(current, end);
					else
						reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->load_midi(current, end);
				}
				else if (name == "PART VOCALS")
					reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9])->load_midi(0, current, end);
				else if (name == "HARM1")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(0, current, end);
				else if (name == "HARM2")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(1, current, end);
				else if (name == "HARM3")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(2, current, end);
			}
			else if (i == 0)
			{
				if (type == 3)
				{
					VariableLengthQuantity length(current);
					if (!m_ini.wasLoaded())
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

	if (drumsLegacy.occupied())
	{
		if (DrumNote_Legacy::isFiveLane())
			DrumTrackConverter::convert(drumsLegacy, reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8]));
		else
			DrumTrackConverter::convert(drumsLegacy, reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7]));
	}
}

void Song::saveFile_Midi(const std::filesystem::path& filepath)
{
	bool useFiveLane = false;
	if (s_noteTracks[7]->occupied() && s_noteTracks[8]->occupied())
	{
		char answer = -1;
		bool loop = true;
		do
		{
			std::cout << "Select Drum Track to save: 4 or 5?\n";
			std::cout << "Answer: ";
			std::cin >> answer;
			std::cin.clear();
			switch (std::tolower(answer))
			{
			case '5':
				useFiveLane = true;
				__fallthrough;
			case '4':
				loop = false;
				break;
			}
		} while (loop);
	}
	else if (s_noteTracks[7]->occupied())
	{
		m_ini.m_pro_drums = true;
		m_ini.m_pro_drum = true;
		m_ini.m_five_lane_drums = false;
	}
	else
	{
		m_ini.m_pro_drums.deactivate();
		m_ini.m_pro_drum.deactivate();

		if (s_noteTracks[8]->occupied())
		{
			m_ini.m_five_lane_drums = true;
			useFiveLane = true;
		}
		else
			m_ini.m_five_lane_drums.deactivate();
	}

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

	if (s_noteTracks[0]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->save_midi("PART GUITAR", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[1]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->save_midi("PART GUITAR GHL", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[2]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->save_midi("PART BASS", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[3]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->save_midi("PART BASS GHL", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[4]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->save_midi("PART RHYTHN", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[5]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->save_midi("PART GUITAR COOP", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[6]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->save_midi("PART KEYS", outFile);
		++header.m_numTracks;
	}
	if (!useFiveLane)
	{
		if (s_noteTracks[7]->occupied())
		{
			reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->save_midi("PART DRUMS", outFile);
			++header.m_numTracks;
		}
	}
	// useFiveLane would only be active if the track is occupied, so no need to check
	else
	{
		reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}
	header.m_numTracks += reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9])->save_midi(outFile);
	header.m_numTracks += reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->save_midi(outFile);

	outFile.seekp(0);
	header.writeToFile(outFile);
	outFile.close();
}
