#include "Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile(MidiTraversal&& traversal)
{
	if (auto starPowerNote = getModifier("star_power_note"))
		NoteTrack::s_starPowerReadNote = (unsigned char)starPowerNote->getValue<uint16_t>();
	else
		NoteTrack::s_starPowerReadNote = 116;

	m_tickrate = traversal.getTickRate();

	Sustainable::setForceThreshold(m_tickrate / 3);
	Sustainable::setsustainThreshold(m_tickrate / 3);

	while (traversal)
	{
		// Checks for a chunk header
		if (traversal.validateChunk())
		{
			const std::string& name = traversal.getTrackName();

			// SyncTrack
			if (traversal.getTrackNumber() == 1)
			{
				if (!m_hasIniFile)
					m_midiSequenceName = UnicodeString::strToU32(name);

				while (traversal.next())
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
							m_sync.back().second.setBPM(60000000.0f / traversal.getMicrosecondsPerQuarter());
						else
						{
							auto& timeSig = traversal.getTimeSig();
							m_sync.back().second.setTimeSig(timeSig.numerator, timeSig.denominator);
						}
					}
				}
			}
			else if (name == "EVENTS")
			{
				while (traversal.next())
				{
					if (traversal.getEventType() < 16)
					{
						std::u32string& text = traversal.getText();
						if (text.compare(0, 8, U"[section") == 0)
						{
							text = text.substr(9, text.length() - 10);
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < traversal.getPosition())
								m_sectionMarkers.push_back({ traversal.getPosition() , std::move(text) });
						}
						else if (text.compare(0, 5, U"[prc_") == 0)
						{
							text = text.substr(5, text.length() - 6);
							if (m_sectionMarkers.empty() || m_sectionMarkers.back().first < traversal.getPosition())
								m_sectionMarkers.push_back({ traversal.getPosition() , std::move(text) });
						}
						else
						{
							if (m_globalEvents.empty() || m_globalEvents.back().first < traversal.getPosition())
							{
								static std::pair<uint32_t, std::vector<std::u32string>> pairNode;
								pairNode.first = traversal.getPosition();
								m_globalEvents.push_back(pairNode);
							}

							m_globalEvents.back().second.push_back(std::move(text));
						}
					}
				}
			}
			else if (name == "PART GUITAR" || name == "T1 GEMS")
				reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0].get())->load_midi(traversal);
			else if (name == "PART GUITAR GHL")
				reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1].get())->load_midi(traversal);
			else if (name == "PART BASS")
				reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2].get())->load_midi(traversal);
			else if (name == "PART BASS GHL")
				reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3].get())->load_midi(traversal);
			else if (name == "PART RHYTHM")
				reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4].get())->load_midi(traversal);
			else if (name == "PART GUITAR COOP")
				reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5].get())->load_midi(traversal);
			else if (name == "PART KEYS")
				reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6].get())->load_midi(traversal);
			else if (name == "PART DRUMS")
			{
				if (TxtFileModifier* fiveLaneDrums = getModifier("five_lane_drums"))
				{
					if (fiveLaneDrums->getValue<bool>())
						reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get())->load_midi(traversal);
					else
						reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get())->load_midi(traversal);
				}
				else
				{
					InstrumentalTrack<DrumNote_Legacy> drumsLegacy;
					drumsLegacy.load_midi(traversal);

					if (!drumsLegacy.isFiveLane())
						*reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get()) = std::move(drumsLegacy);
					else
						*reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get()) = std::move(drumsLegacy);
				}
			}
			else if (name == "PART VOCALS")
				reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9].get())->load_midi<0>(traversal);
			else if (name == "HARM1")
				reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10].get())->load_midi<0>(traversal);
			else if (name == "HARM2")
				reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10].get())->load_midi<1>(traversal);
			else if (name == "HARM3")
				reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10].get())->load_midi<2>(traversal);
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
	MidiChunk_Header header(m_tickrate);
	header.writeToFile(outFile);

	MidiChunk_Track sync;
	if (!m_midiSequenceName.empty())
		sync.addEvent(0, new MidiChunk_Track::MetaEvent_Text(3, UnicodeString::U32ToStr(m_midiSequenceName)));

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
			events.addEvent(eventIter->first, new MidiChunk_Track::MetaEvent_Text(1, UnicodeString::U32ToStr(str)));
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
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[0].get())->save_midi("PART GUITAR", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[1]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[1].get())->save_midi("PART GUITAR GHL", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[2]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[2].get())->save_midi("PART BASS", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[3]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<6>>*>(s_noteTracks[3].get())->save_midi("PART BASS GHL", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[4]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[4].get())->save_midi("PART RHYTHN", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[5]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<GuitarNote<5>>*>(s_noteTracks[5].get())->save_midi("PART GUITAR COOP", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[6]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<Keys<5>>*>(s_noteTracks[6].get())->save_midi("PART KEYS", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[7]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<DrumNote<4, DrumPad_Pro>>*>(s_noteTracks[7].get())->save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}
	if (s_noteTracks[8]->occupied())
	{
		reinterpret_cast<InstrumentalTrack<DrumNote<5, DrumPad>>*>(s_noteTracks[8].get())->save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}
	header.m_numTracks += reinterpret_cast<VocalTrack<1>*>(s_noteTracks[9].get())->save_midi(outFile);
	header.m_numTracks += reinterpret_cast<VocalTrack<3>*>(s_noteTracks[10].get())->save_midi(outFile);

	outFile.seekp(0);
	header.writeToFile(outFile);
	outFile.close();
}
