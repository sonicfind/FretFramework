#include "Song/Song.h"
#include "Tracks/VocalTracks/VocalTrack_midi.hpp"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::loadFile(MidiTraversal&& traversal)
{
	if (auto starPowerNote = m_currentSongEntry->getModifier("star_power_note"))
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
				if (!m_currentSongEntry->hasIniFile())
					m_midiSequenceName = UnicodeString::strToU32(name);

				while (traversal.next())
				{
					if (traversal.getEventType() == 0x51 || traversal.getEventType() == 0x58)
					{
						// Starts the values at the current location with the previous set of values
						if (m_sync.back().first < traversal.getPosition())
							m_sync.push_back({ traversal.getPosition(), m_sync.back().second.copy()});

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
				m_noteTracks.lead_5.load_midi(traversal);
			else if (name == "PART GUITAR GHL")
				m_noteTracks.lead_6.load_midi(traversal);
			else if (name == "PART BASS")
				m_noteTracks.bass_5.load_midi(traversal);
			else if (name == "PART BASS GHL")
				m_noteTracks.bass_6.load_midi(traversal);
			else if (name == "PART RHYTHM")
				m_noteTracks.rhythm.load_midi(traversal);
			else if (name == "PART GUITAR COOP")
				m_noteTracks.coop.load_midi(traversal);
			else if (name == "PART KEYS")
				m_noteTracks.keys.load_midi(traversal);
			else if (name == "PART DRUMS")
			{
				if (TxtFileModifier* fiveLaneDrums = m_currentSongEntry->getModifier("five_lane_drums"))
				{
					if (fiveLaneDrums->getValue<bool>())
						m_noteTracks.drums5.load_midi(traversal);
					else
						m_noteTracks.drums4_pro.load_midi(traversal);
				}
				else
				{
					InstrumentalTrack<DrumNote_Legacy> drumsLegacy;
					drumsLegacy.load_midi(traversal);

					if (!drumsLegacy.isFiveLane())
						m_noteTracks.drums4_pro = std::move(drumsLegacy);
					else
						m_noteTracks.drums5 = std::move(drumsLegacy);
				}
			}
			else if (name == "PART VOCALS")
				m_noteTracks.vocals.load_midi<0>(traversal);
			else if (name == "HARM1")
				m_noteTracks.harmonies.load_midi<0>(traversal);
			else if (name == "HARM2")
				m_noteTracks.harmonies.load_midi<1>(traversal);
			else if (name == "HARM3")
				m_noteTracks.harmonies.load_midi<2>(traversal);
		}
		else
			traversal.setNextTrack(traversal.findNextChunk());
		traversal.skipTrack();
	}
}

void Song::saveFile_Midi() const
{
	std::fstream outFile = FilestreamCheck::getFileStream(m_currentSongEntry->getFilePath(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
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

	if (m_noteTracks.lead_5.occupied())
	{
		m_noteTracks.lead_5.save_midi("PART GUITAR", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.lead_6.occupied())
	{
		m_noteTracks.lead_6.save_midi("PART GUITAR GHL", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.bass_5.occupied())
	{
		m_noteTracks.bass_5.save_midi("PART BASS", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.bass_6.occupied())
	{
		m_noteTracks.bass_6.save_midi("PART BASS GHL", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.rhythm.occupied())
	{
		m_noteTracks.rhythm.save_midi("PART RHYTHN", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.coop.occupied())
	{
		m_noteTracks.coop.save_midi("PART GUITAR COOP", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.keys.occupied())
	{
		m_noteTracks.keys.save_midi("PART KEYS", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.drums4_pro.occupied())
	{
		m_noteTracks.drums4_pro.save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}
	if (m_noteTracks.drums5.occupied())
	{
		m_noteTracks.drums5.save_midi("PART DRUMS", outFile);
		++header.m_numTracks;
	}
	header.m_numTracks += m_noteTracks.vocals.save_midi(outFile);
	header.m_numTracks += m_noteTracks.harmonies.save_midi(outFile);

	outFile.seekp(0);
	header.writeToFile(outFile);
	outFile.close();
}
