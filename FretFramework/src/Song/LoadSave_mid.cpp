#include "Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile_Midi()
{
	if (m_ini.m_star_power_note != 116)
		NoteTrack::s_starPowerReadNote = (unsigned char)m_ini.m_multiplier_note;
	else if (m_ini.m_multiplier_note != 116)
		NoteTrack::s_starPowerReadNote = (unsigned char)m_ini.m_star_power_note;
	else
		NoteTrack::s_starPowerReadNote = 116;

	MidiTraversal traversal(m_fullPath);
	m_tickrate = traversal.getTickRate();

	if (m_ini.m_eighthnote_hopo)
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 2);
	else
		m_ini.m_hopo_frequency.setDefault(m_tickrate / 3);

	m_ini.m_sustain_cutoff_threshold.setDefault(m_tickrate / 3);
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	while (traversal)
	{
		// Checks for a chunk header
		if (traversal.validateChunk())
		{
			if (traversal.next() && traversal.getEventType() < 128 && traversal.getEventType() != 0x2F)
			{
				std::string name;
				if (traversal.getEventType() == 3)
					name = traversal.extractText();

				// SyncTrack
				if (traversal.getTrackNumber() == 1)
				{
					if (!name.empty())
					{
						if (!m_ini.wasLoaded())
							m_songInfo.name = name;

						if (!traversal.next() || traversal.getEventType() == 0x2F)
							continue;
					}

					do
					{
						if (traversal.getEventType() == 0x51 || traversal.getEventType() == 0x58)
						{
							// Starts the values at the current location with the previous set of values
							if (m_sync.back().first < traversal.getPosition())
							{
								static SyncValues prev;
								prev = m_sync.back().second;
								m_sync.push_back({ traversal.getPosition(), prev });
							}

							if (traversal.getEventType() == 0x51)
							{
								uint32_t microsecondsPerQuarter = 0;
								memcpy((char*)&microsecondsPerQuarter + 1, traversal.getCurrent(), 3);
								m_sync.back().second.setBPM(60000000.0f / _byteswap_ulong(microsecondsPerQuarter));
							}
							else
								m_sync.back().second.setTimeSig(traversal[0], traversal[1]);
						}
					} while (traversal.next() && traversal.getEventType() != 0x2F);
				}
				else if (name == "EVENTS")
				{
					while (traversal.next() && traversal.getEventType() != 0x2F)
					{
						if (traversal.getEventType() < 16)
						{
							std::string text = traversal.extractText();
							bool section = false;
							if (strncmp(text.data(), "[section", 8) == 0)
							{
								text = text.substr(9, text.length() - 10);
								section = true;
							}
							else if (strncmp(text.data(), "[prc_", 5) == 0)
							{
								text = text.substr(5, text.length() - 6);
								section = true;
							}

							if (section)
							{
								if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < traversal.getPosition())
									m_sectionMarkers.push_back({ traversal.getPosition() , text });
							}
							else
							{
								if (m_globalEvents.empty() || m_globalEvents.back().first < traversal.getPosition())
								{
									static std::pair<uint32_t, std::vector<UnicodeString>> pairNode;
									pairNode.first = traversal.getPosition();
									m_globalEvents.push_back(pairNode);
								}

								m_globalEvents.back().second.push_back(text);
							}
						}
					}
				}
				else if (name == "PART GUITAR" || name == "T1 GEMS")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0])->load_midi(traversal);
				else if (name == "PART GUITAR GHL")
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1])->load_midi(traversal);
				else if (name == "PART BASS")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2])->load_midi(traversal);
				else if (name == "PART BASS GHL")
					reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3])->load_midi(traversal);
				else if (name == "PART RHYTHM")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4])->load_midi(traversal);
				else if (name == "PART GUITAR COOP")
					reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5])->load_midi(traversal);
				else if (name == "PART KEYS")
					reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6])->load_midi(traversal);
				else if (name == "PART DRUMS")
				{
					if (!m_ini.m_five_lane_drums.isActive())
					{
						InstrumentalTrack<DrumNote_Legacy> drumsLegacy;
						drumsLegacy.load_midi(traversal);

						if (!drumsLegacy.isFiveLane())
							*reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7]) = drumsLegacy;
						else
							*reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8]) = drumsLegacy;
					}
					else if (!m_ini.m_five_lane_drums)
						reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->load_midi(traversal);
					else
						reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8])->load_midi(traversal);
				}
				else if (name == "PART VOCALS")
					reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9])->load_midi(0, traversal);
				else if (name == "HARM1")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(0, traversal);
				else if (name == "HARM2")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(1, traversal);
				else if (name == "HARM3")
					reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10])->load_midi(2, traversal);
			}
		}
		else
			traversal.setNextTrack(traversal.findNextChunk());
		traversal.skipTrack();
	}
}

void Song::saveFile_Midi() const
{
	std::filesystem::path filepath = m_directory;
	filepath += m_chartFile;

	std::fstream outFile = FilestreamCheck::getFileStream(filepath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	MidiChunk_Header header(m_tickrate.m_value);
	header.writeToFile(outFile);

	MidiChunk_Track sync;
	if (!m_songInfo.name.m_value->empty())
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
			events.addEvent(sectIter->first, new MidiChunk_Track::MetaEvent_Text(1, "[section " + sectIter->second.toString() + ']'));
			++sectIter;
		}

		for (const auto& str : eventIter->second)
			events.addEvent(eventIter->first, new MidiChunk_Track::MetaEvent_Text(1, str));
	}

	while (sectIter != m_sectionMarkers.end())
	{
		events.addEvent(sectIter->first, new MidiChunk_Track::MetaEvent_Text(1, "[section " + sectIter->second.toString() + ']'));
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
	if (!m_ini.m_five_lane_drums)
	{
		if (s_noteTracks[7]->occupied())
		{
			reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7])->save_midi("PART DRUMS", outFile);
			++header.m_numTracks;
		}
	}
	else if (s_noteTracks[8]->occupied())
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
