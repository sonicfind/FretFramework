#pragma once
#include <array>
#include "../../NodeTrack.h"

class MidiTrackWriter
{
protected:
	MidiFile::MidiChunk_Track m_events;
	MidiTrackWriter(const std::string& name);

public:
	void writeToFile(std::fstream& outFile);
};

template <typename N>
class MidiTrackFiller : public MidiTrackWriter
{
	uint32_t m_sliderNotes = UINT32_MAX;
	std::pair<uint32_t, const N*> m_prevNote[5] = {};
	std::map<uint32_t, std::array<const N*, 5>> m_notes;

	// Functions that will be overriden by specializations
	void insertNoteEvents();
	void processNote(uint32_t position, const N* note, int difficulty);

public:
	MidiTrackFiller(const std::string& name, const NodeTrack<N>& track)
		: MidiTrackWriter(name)
	{
		for (const auto& vec : track.m_difficulties[0].m_events)
			for (const auto& ev : vec.second)
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

		for (const auto& vec : track.m_difficulties[0].m_soloes)
		{
			m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 103));
			m_events.addEvent(vec.first + vec.second, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 103, 0));
		}

		for (const auto& vec : track.m_difficulties[0].m_effects)
			for (const auto& effect : vec.second)
			{
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}

		for (int difficulty = 0; difficulty < 4; ++difficulty)
			for (const auto& node : track.m_difficulties[difficulty].m_notes)
				m_notes[node.first][3ULL - difficulty] = &node.second;

		// BRE
		for (const auto& node : track.m_difficulties[4].m_notes)
			m_notes[node.first][4] = &node.second;
		
		if (track.hasNotes())
			insertNoteEvents();
	}
};

template <>
class MidiTrackFiller<DrumNote> : public MidiTrackWriter
{
	uint32_t m_toms[3] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };
	bool m_useDynamics = false;
	std::map<uint32_t, std::array<const DrumNote*, 5>> m_notes;
	std::vector<std::pair<uint32_t, uint32_t>> m_fills;

public:
	MidiTrackFiller(const std::string& name, const NodeTrack<DrumNote>& track);
};

template<>
void MidiTrackFiller<GuitarNote<5>>::insertNoteEvents();

template<>
void MidiTrackFiller<GuitarNote<5>>::processNote(uint32_t position, const GuitarNote<5>* note, int difficulty);

template<>
void MidiTrackFiller<GuitarNote<6>>::insertNoteEvents();

template<>
void MidiTrackFiller<GuitarNote<6>>::processNote(uint32_t position, const GuitarNote<6>* note, int difficulty);
