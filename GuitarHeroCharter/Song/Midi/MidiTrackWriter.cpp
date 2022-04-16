#include "MidiTrackWriter.h"

MidiTrackWriter::MidiTrackWriter(const std::string& name)
	: m_events(name)
{
}

void MidiTrackWriter::writeToFile(std::fstream& outFile)
{
	m_events.writeToFile(outFile);
}

template<>
void MidiTrackFiller<GuitarNote<5>>::insertNoteEvents()
{
	m_events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENHANCED_OPENS]"));
	for (const auto& node : m_notes)
	{
		// Only define tap segments by the expert track
		if (node.second[3] != nullptr)
		{
			if (node.second[3]->m_isTap)
			{
				// NoteOn
				if (m_sliderNotes == UINT32_MAX)
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
				m_sliderNotes = node.first;
			}
			else if (m_sliderNotes != UINT32_MAX)
			{
				if (m_sliderNotes <= node.first)
					// The previous note ended, so we can attach the NoteOff to its end
					m_events.addEvent(m_sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				else
					// This note cuts off the slider event earlier than expected
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				m_sliderNotes = UINT32_MAX;
			}
		}

		for (int difficulty = 0; difficulty < 4; ++difficulty)
		{
			if (node.second[difficulty] != nullptr)
			{
				processNote(node.first, node.second[difficulty], difficulty);
				m_prevNote[difficulty].first = node.first;
				m_prevNote[difficulty].second = node.second[difficulty];
			}
		}
	}

	// Add the NoteOff event for a remaining slider phrase
	if (m_sliderNotes != UINT32_MAX)
	{
		m_events.addEvent(m_sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
		m_sliderNotes = UINT32_MAX;
	}
}

template<>
void MidiTrackFiller<GuitarNote<5>>::processNote(uint32_t position, const GuitarNote<5>* note, int difficulty)
{
	const uint32_t base = difficulty != 4 ? 59 + 12 * difficulty : 119;
	auto placeNote = [&](char note, uint32_t sustain)
	{
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note));
		if (sustain == 0)
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, 0));
		else
			m_events.addEvent(position + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, 0));
	};

	if (note->m_open)
		placeNote(base, note->m_open.getSustain());
	else
	{
		for (char col = 0; col < 5; ++col)
			if (note->m_colors[col])
				placeNote(base + col + 1, note->m_colors[col].getSustain());
	}

	switch (note->m_isForced)
	{
	case GuitarNote<5>::ForceStatus::HOPO_ON:
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6));
		m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
		break;
	case GuitarNote<5>::ForceStatus::HOPO_OFF:
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7));
		m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
		break;
	case GuitarNote<5>::ForceStatus::FORCED:
		// Naturally a hopo, so add Forced HOPO Off
		if (note->getNumActiveColors() < 2 && position <= m_prevNote[difficulty].first + Hittable::getForceThreshold())
		{
			m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7));
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
		}
		// Naturally a strum, so add Forced HOPO On
		else
		{
			m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6));
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
		}
	}
	
	// To properly place the NoteOff for a slider event, we need to know
	// what the longest sustain in this note is
	// 
	// Notice: while the actual event turning off and on is notated only by the expert track,
	// *when* it ends can be determined by any note within it
	if (m_sliderNotes != UINT32_MAX)
	{
		uint32_t sustain = 0;
		if (note->m_open)
			sustain = note->m_open.getSustain();
		else
		{
			for (const auto& color : note->m_colors)
				if (color && color.getSustain() > sustain)
					sustain = color.getSustain();
		}

		if (sustain == 0)
			sustain = 1;

		if (m_sliderNotes < position + sustain)
			m_sliderNotes = position + sustain;
	}
}

template<>
void MidiTrackFiller<GuitarNote<6>>::insertNoteEvents()
{
	for (const auto& node : m_notes)
	{
		// Only define tap segments by the expert track
		if (node.second[3] != nullptr)
		{
			if (node.second[3]->m_isTap)
			{
				if (m_sliderNotes == UINT32_MAX)
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
				m_sliderNotes = node.first;
			}
			else if (m_sliderNotes != UINT32_MAX)
			{
				if (m_sliderNotes <= node.first)
					// The previous note ended, so we can attach the NoteOff to its end
					m_events.addEvent(m_sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				else
					// This note cuts off the slider event earlier than expected
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
				m_sliderNotes = UINT32_MAX;
			}
		}

		for (int difficulty = 0; difficulty < 4; ++difficulty)
		{
			if (node.second[difficulty] != nullptr)
			{
				processNote(node.first, node.second[difficulty], difficulty);
				m_prevNote[difficulty].first = node.first;
				m_prevNote[difficulty].second = node.second[difficulty];
			}
		}
	}

	// Add the NoteOff event for a remaining slider phrase
	if (m_sliderNotes != UINT32_MAX)
	{
		m_events.addEvent(m_sliderNotes, new MidiFile::MidiChunk_Track::SysexEvent(0xFF, 4, 0));
		m_sliderNotes = UINT32_MAX;
	}
}

template<>
void MidiTrackFiller<GuitarNote<6>>::processNote(uint32_t position, const GuitarNote<6>* note, int difficulty)
{
	const uint32_t base = difficulty != 4 ? 58 + 12 * difficulty : 119;
	auto placeNote = [&](char note, uint32_t sustain)
	{
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note));
		if (sustain == 0)
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, 0));
		else
			m_events.addEvent(position + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, 0));
	};

	if (note->m_open)
		placeNote(base, note->m_open.getSustain());
	else
	{
		for (char col = 0; col < 6; ++col)
			if (note->m_colors[col])
			{
				// Black and white midi notes are in swapped order for some reason
				if (col < 3)
					placeNote(base + col + 3 + 1, note->m_colors[col].getSustain());
				else
					placeNote(base + col - 3 + 1, note->m_colors[col].getSustain());
			}
	}

	switch (note->m_isForced)
	{
	case GuitarNote<6>::ForceStatus::HOPO_ON:
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6));
		m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
		break;
	case GuitarNote<6>::ForceStatus::HOPO_OFF:
		m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7));
		m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
		break;
	case GuitarNote<6>::ForceStatus::FORCED:
		// Naturally a hopo, so add Forced HOPO Off
		if (note->getNumActiveColors() < 2 && position <= m_prevNote[difficulty].first + Hittable::getForceThreshold())
		{
			m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7));
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 7, 0));
		}
		// Naturally a strum, so add Forced HOPO On
		else
		{
			m_events.addEvent(position, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6));
			m_events.addEvent(position + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, base + 6, 0));
		}
	}

	// To properly place the NoteOff for a slider event, we need to know
	// what the longest sustain in this note is
	// 
	// Notice: while the actual event turning off and on is notated only by the expert track,
	// *when* it ends can be determined by any note within it
	if (m_sliderNotes != UINT32_MAX)
	{
		uint32_t sustain = 0;
		if (note->m_open)
			sustain = note->m_open.getSustain();
		else
		{
			for (const auto& color : note->m_colors)
				if (color && color.getSustain() > sustain)
					sustain = color.getSustain();
		}

		if (sustain == 0)
			sustain = 1;

		if (m_sliderNotes < position + sustain)
			m_sliderNotes = position + sustain;
	}
}

MidiTrackFiller<DrumNote>::MidiTrackFiller(const std::string& name, const NodeTrack<DrumNote>& track)
	: MidiTrackWriter(name)
{
	// Insert text events
	for (const auto& vec : track.m_difficulties[0].m_events)
		for (const auto& ev : vec.second)
			m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

	// Insert soloes
	for (const auto& vec : track.m_difficulties[0].m_soloes)
	{
		m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 103));
		m_events.addEvent(vec.first + vec.second, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 103, 0));
	}
	
	// Insert effects and save fills
	for (const auto& vec : track.m_difficulties[0].m_effects)
		for (const auto& effect : vec.second)
		{
			if (effect->getMidiNote() != -1)
			{
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}
			else
				m_fills.push_back({ vec.first, vec.first + effect->getDuration() });
		}

	for (int difficulty = 0; difficulty < 4; ++difficulty)
		for (const auto& node : track.m_difficulties[difficulty].m_notes)
			m_notes[node.first][3ULL - difficulty] = &node.second;

	// BRE
	for (const auto& node : track.m_difficulties[4].m_notes)
		m_notes[node.first][4] = &node.second;

	// Insert note events

	for (const auto& node : m_notes)
	{
		// Need to test for the appropriate statuses of toms
		for (int lane = 0; lane < 3; ++lane)
		{
			for (int difficulty = 4; difficulty >= 0; --difficulty)
			{
				if (node.second[difficulty] != nullptr && node.second[difficulty]->m_colors[1 + lane])
				{
					if (node.second[difficulty]->m_colors[1 + lane].m_isCymbal)
					{
						// Ensure that toms are disabled
						if (m_toms[lane] != UINT32_MAX)
						{
							// Create Note off Event for the tom marker
							m_events.addEvent(m_toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
							m_toms[lane] = UINT32_MAX;
						}
					}
					else
					{
						// Ensure toms are enabled
						if (m_toms[lane] == UINT32_MAX)
							// Create Note On Event for the tom marker
							m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane));
						m_toms[lane] = node.first + 1;
					}
					break;
				}
			}
		}

		for (int difficulty = 0; difficulty < 4; ++difficulty)
		{
			if (node.second[difficulty] != nullptr)
			{
				const uint32_t base = difficulty != 4 ? 60 + 12 * difficulty : 119;
				auto placeNote = [&](char note, char velocity)
				{
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, velocity));
					m_events.addEvent(node.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, note, 0));
				};

				if (node.second[difficulty]->m_open)
				{
					if (node.second[difficulty]->m_open.m_isDoubleBass)
						placeNote(95, 100);
					else
						placeNote(base, 100);
				}

				for (char col = 0; col < 4; ++col)
					if (node.second[difficulty]->m_colors[col])
					{
						if (node.second[difficulty]->m_colors[col].m_isAccented)
						{
							// For now, use 127 as the default accent velocity
							placeNote(base + col + 1, 127);
							m_useDynamics = true;
						}
						else if (node.second[difficulty]->m_colors[col].m_isGhosted)
						{
							// For now, use 1 as the default accent velocity
							placeNote(base + col + 1, 1);
							m_useDynamics = true;
						}
						else
							placeNote(base + col + 1, 100);
					}

				if (node.second[difficulty]->m_fifthLane)
				{
					if (node.second[difficulty]->m_fifthLane.m_isAccented)
					{
						// For now, use 127 as the default accent velocity
						placeNote(base + 5, 127);
						m_useDynamics = true;
					}
					else if (node.second[difficulty]->m_fifthLane.m_isGhosted)
					{
						// For now, use 1 as the default accent velocity
						placeNote(base + 5, 1);
						m_useDynamics = true;
					}
					else
						placeNote(base + 5, 100);
				}

				// Only the expert track controls flams in .mid
				if (difficulty == 3 && node.second[difficulty]->m_isFlamed)
				{
					m_events.addEvent(node.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
					m_events.addEvent(node.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
				}
			}
		}
	}

	const char numLanes = (char)DrumNote::getLaneSize() + 1;
	for (const auto& fill : m_fills)
	{
		for (char lane = 0; lane < numLanes; ++lane)
		{
			m_events.addEvent(fill.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 120 + lane));
			m_events.addEvent(fill.second, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 120 + lane, 0));
		}
	}

	// Disable any active tom markers
	for (int lane = 0; lane < 3; ++lane)
		if (m_toms[lane] != UINT32_MAX)
		{
			// Create Note off Event for the tom marker
			m_events.addEvent(m_toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
			m_toms[lane] = UINT32_MAX;
		}

	if (m_useDynamics)
		m_events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENABLE_CHART_DYNAMICS]"));
}
