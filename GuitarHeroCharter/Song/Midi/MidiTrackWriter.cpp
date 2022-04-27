#include "MidiTrackWriter.h"

MidiTrackWriter::MidiTrackWriter(const std::string& name)
	: m_events(name)
{
}

void MidiTrackWriter::writeToFile(std::fstream& outFile)
{
	if (m_doWrite)
		m_events.writeToFile(outFile);
}

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<GuitarNote<5>>& track)
{
	for (const auto& vec : track.m_difficulties[0].m_events)
		for (const auto& ev : vec.second)
			m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

	for (const auto& vec : track.m_difficulties[0].m_effects)
		for (const auto& effect : vec.second)
			if (effect->getMidiNote() != -1)
			{
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}
			else
				for (int lane = 120; lane < 125; ++lane)
				{
					m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane));
					m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane, 0));
				}

	if (!track.hasNotes())
		return;

	m_doWrite = true;
	m_events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENHANCED_OPENS]"));

	uint32_t sliderNotes = UINT32_MAX;
	auto processNote = [&](const std::pair<uint32_t, GuitarNote<5>>& note,
							char baseMidiNote,
							const std::pair<uint32_t, GuitarNote<5>>* const prev)
	{
		auto placeNote = [&](char midiNote, uint32_t sustain)
		{
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
			if (sustain == 0)
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			else
				m_events.addEvent(note.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
		};

		if (note.second.m_open)
			placeNote(baseMidiNote, note.second.m_open.getSustain());
		else
		{
			for (char col = 0; col < 5; ++col)
				if (note.second.m_colors[col])
					placeNote(baseMidiNote + col + 1, note.second.m_colors[col].getSustain());
		}

		switch (note.second.m_isForced)
		{
		case GuitarNote<5>::ForceStatus::HOPO_ON:
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
			m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
			break;
		case GuitarNote<5>::ForceStatus::HOPO_OFF:
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
			m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
			break;
		case GuitarNote<5>::ForceStatus::FORCED:
			// Naturally a hopo, so add Forced HOPO Off
			if (note.second.getNumActiveColors() < 2 &&
				prev &&
				note.first <= prev->first + Hittable::getForceThreshold())
			{
				m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
			}
			// Naturally a strum, so add Forced HOPO On
			else
			{
				m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
			}
		}

		// To properly place the NoteOff for a slider event, we need to know
		// what the longest sustain in this note is
		// 
		// Notice: while the actual event turning off and on is notated only by the expert track,
		// *when* it ends can be determined by any note within it
		if (sliderNotes != UINT32_MAX)
		{
			uint32_t sustain = 0;
			if (note.second.m_open)
				sustain = note.second.m_open.getSustain();
			else
			{
				for (const auto& color : note.second.m_colors)
					if (color && color.getSustain() > sustain)
						sustain = color.getSustain();
			}

			if (sustain == 0)
				sustain = 1;

			if (sliderNotes < note.first + sustain)
				sliderNotes = note.first + sustain;
		}
	};

	auto expertIter = track.m_difficulties[0].m_notes.begin();
	auto hardIter = track.m_difficulties[1].m_notes.begin();
	auto mediumIter = track.m_difficulties[2].m_notes.begin();
	auto easyIter = track.m_difficulties[3].m_notes.begin();
	bool expertValid = expertIter != track.m_difficulties[0].m_notes.end();
	bool hardValid = hardIter != track.m_difficulties[1].m_notes.end();
	bool mediumValid = mediumIter != track.m_difficulties[2].m_notes.end();
	bool easyValid = easyIter != track.m_difficulties[3].m_notes.end();

	int sliderDifficulty = 0;
	if (!expertValid)
		++sliderDifficulty;
	if (!hardValid)
		++sliderDifficulty;
	if (!mediumValid)
		++sliderDifficulty;

	auto adjustSlider = [&](const std::pair<uint32_t, GuitarNote<5>>& pair)
	{
		if (pair.second.m_isTap)
		{
			// NoteOn
			if (sliderNotes == UINT32_MAX)
				m_events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
			sliderNotes = pair.first;
		}
		else if (sliderNotes != UINT32_MAX)
		{
			if (sliderNotes <= pair.first)
				// The previous note ended, so we can attach the NoteOff to its end
				m_events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
			else
				// This note cuts off the slider event earlier than expected
				m_events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
			sliderNotes = UINT32_MAX;
		}
	};

	while (expertValid || hardValid || mediumValid || easyValid)
	{
		while (expertValid &&
			(!hardValid || expertIter->first <= hardIter->first) &&
			(!mediumValid || expertIter->first <= mediumIter->first) &&
			(!easyValid || expertIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 0)
				adjustSlider(*expertIter);

			if (expertIter != track.m_difficulties[0].m_notes.begin())
				processNote(*expertIter, 95, (expertIter - 1)._Ptr);
			else
				processNote(*expertIter, 95, nullptr);
			expertValid = ++expertIter != track.m_difficulties[0].m_notes.end();
		}

		while (hardValid &&
			(!expertValid || hardIter->first < expertIter->first) &&
			(!mediumValid || hardIter->first <= mediumIter->first) &&
			(!easyValid || hardIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 1)
				adjustSlider(*hardIter);

			if (hardIter != track.m_difficulties[1].m_notes.begin())
				processNote(*hardIter, 83, (hardIter - 1)._Ptr);
			else
				processNote(*hardIter, 83, nullptr);
			hardValid = ++hardIter != track.m_difficulties[1].m_notes.end();
		}

		while (mediumValid &&
			(!expertValid || mediumIter->first < expertIter->first) &&
			(!hardValid || mediumIter->first < hardIter->first) &&
			(!easyValid || mediumIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 2)
				adjustSlider(*mediumIter);

			if (mediumIter != track.m_difficulties[2].m_notes.begin())
				processNote(*mediumIter, 71, (mediumIter - 1)._Ptr);
			else
				processNote(*mediumIter, 71, nullptr);
			mediumValid = ++mediumIter != track.m_difficulties[2].m_notes.end();
		}

		while (easyValid &&
			(!expertValid || easyIter->first < expertIter->first) &&
			(!hardValid || easyIter->first < hardIter->first) &&
			(!mediumValid || easyIter->first < mediumIter->first))
		{
			if (sliderDifficulty == 3)
				adjustSlider(*easyIter);

			if (easyIter != track.m_difficulties[3].m_notes.begin())
				processNote(*easyIter, 59, (easyIter - 1)._Ptr);
			else
				processNote(*easyIter, 59, nullptr);
			easyValid = ++easyIter != track.m_difficulties[3].m_notes.end();
		}
	}

	// Add the NoteOff event for a remaining slider phrase
	if (sliderNotes != UINT32_MAX)
	{
		m_events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
		sliderNotes = UINT32_MAX;
	}
}

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<GuitarNote<6>>& track)
{
	for (const auto& vec : track.m_difficulties[0].m_events)
		for (const auto& ev : vec.second)
			m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

	for (const auto& vec : track.m_difficulties[0].m_effects)
		for (const auto& effect : vec.second)
			if (effect->getMidiNote() != -1)
			{
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}
			else
				for (int lane = 120; lane < 125; ++lane)
				{
					m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane));
					m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane, 0));
				}

	if (!track.hasNotes())
		return;

	m_doWrite = true;

	uint32_t sliderNotes = UINT32_MAX;
	auto processNote = [&](const std::pair<uint32_t, GuitarNote<6>>& note,
		char baseMidiNote,
		const std::pair<uint32_t, GuitarNote<6>>* const prev)
	{
		auto placeNote = [&](char midiNote, uint32_t sustain)
		{
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote));
			if (sustain == 0)
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
			else
				m_events.addEvent(note.first + sustain, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
		};

		if (note.second.m_open)
			placeNote(baseMidiNote, note.second.m_open.getSustain());
		else
		{
			for (char col = 0; col < 5; ++col)
				if (note.second.m_colors[col])
					placeNote(baseMidiNote + col + 1, note.second.m_colors[col].getSustain());
		}

		switch (note.second.m_isForced)
		{
		case GuitarNote<6>::ForceStatus::HOPO_ON:
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
			m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
			break;
		case GuitarNote<6>::ForceStatus::HOPO_OFF:
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
			m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
			break;
		case GuitarNote<6>::ForceStatus::FORCED:
			// Naturally a hopo, so add Forced HOPO Off
			if (note.second.getNumActiveColors() < 2 &&
				prev &&
				note.first <= prev->first + Hittable::getForceThreshold())
			{
				m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7));
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 7, 0));
			}
			// Naturally a strum, so add Forced HOPO On
			else
			{
				m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6));
				m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, baseMidiNote + 6, 0));
			}
		}

		// To properly place the NoteOff for a slider event, we need to know
		// what the longest sustain in this note is
		// 
		// Notice: while the actual event turning off and on is notated only by the expert track,
		// *when* it ends can be determined by any note within it
		if (sliderNotes != UINT32_MAX)
		{
			uint32_t sustain = 0;
			if (note.second.m_open)
				sustain = note.second.m_open.getSustain();
			else
			{
				for (const auto& color : note.second.m_colors)
					if (color && color.getSustain() > sustain)
						sustain = color.getSustain();
			}

			if (sustain == 0)
				sustain = 1;

			if (sliderNotes < note.first + sustain)
				sliderNotes = note.first + sustain;
		}
	};

	auto expertIter = track.m_difficulties[0].m_notes.begin();
	auto hardIter = track.m_difficulties[1].m_notes.begin();
	auto mediumIter = track.m_difficulties[2].m_notes.begin();
	auto easyIter = track.m_difficulties[3].m_notes.begin();
	bool expertValid = expertIter != track.m_difficulties[0].m_notes.end();
	bool hardValid = hardIter != track.m_difficulties[1].m_notes.end();
	bool mediumValid = mediumIter != track.m_difficulties[2].m_notes.end();
	bool easyValid = easyIter != track.m_difficulties[3].m_notes.end();

	int sliderDifficulty = 0;
	if (!expertValid)
		++sliderDifficulty;
	if (!hardValid)
		++sliderDifficulty;
	if (!mediumValid)
		++sliderDifficulty;

	auto adjustSlider = [&](const std::pair<uint32_t, GuitarNote<6>>& pair)
	{
		if (pair.second.m_isTap)
		{
			// NoteOn
			if (sliderNotes == UINT32_MAX)
				m_events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104));
			sliderNotes = pair.first;
		}
		else if (sliderNotes != UINT32_MAX)
		{
			if (sliderNotes <= pair.first)
				// The previous note ended, so we can attach the NoteOff to its end
				m_events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
			else
				// This note cuts off the slider event earlier than expected
				m_events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 104, 0));
			sliderNotes = UINT32_MAX;
		}
	};

	while (expertValid || hardValid || mediumValid || easyValid)
	{
		while (expertValid &&
			(!hardValid || expertIter->first <= hardIter->first) &&
			(!mediumValid || expertIter->first <= mediumIter->first) &&
			(!easyValid || expertIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 0)
				adjustSlider(*expertIter);

			if (expertIter != track.m_difficulties[0].m_notes.begin())
				processNote(*expertIter, 94, (expertIter - 1)._Ptr);
			else
				processNote(*expertIter, 94, nullptr);
			expertValid = ++expertIter != track.m_difficulties[0].m_notes.end();
		}

		while (hardValid &&
			(!expertValid || hardIter->first < expertIter->first) &&
			(!mediumValid || hardIter->first <= mediumIter->first) &&
			(!easyValid || hardIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 1)
				adjustSlider(*hardIter);

			if (hardIter != track.m_difficulties[1].m_notes.begin())
				processNote(*hardIter, 82, (hardIter - 1)._Ptr);
			else
				processNote(*hardIter, 82, nullptr);
			hardValid = ++hardIter != track.m_difficulties[1].m_notes.end();
		}

		while (mediumValid &&
			(!expertValid || mediumIter->first < expertIter->first) &&
			(!hardValid || mediumIter->first < hardIter->first) &&
			(!easyValid || mediumIter->first <= easyIter->first))
		{
			if (sliderDifficulty == 2)
				adjustSlider(*mediumIter);

			if (mediumIter != track.m_difficulties[2].m_notes.begin())
				processNote(*mediumIter, 70, (mediumIter - 1)._Ptr);
			else
				processNote(*mediumIter, 70, nullptr);
			mediumValid = ++mediumIter != track.m_difficulties[2].m_notes.end();
		}

		while (easyValid &&
			(!expertValid || easyIter->first < expertIter->first) &&
			(!hardValid || easyIter->first < hardIter->first) &&
			(!mediumValid || easyIter->first < mediumIter->first))
		{
			if (sliderDifficulty == 3)
				adjustSlider(*easyIter);

			if (easyIter != track.m_difficulties[3].m_notes.begin())
				processNote(*easyIter, 58, (easyIter - 1)._Ptr);
			else
				processNote(*easyIter, 58, nullptr);
			easyValid = ++easyIter != track.m_difficulties[3].m_notes.end();
		}
	}

	// Add the NoteOff event for a remaining slider phrase
	if (sliderNotes != UINT32_MAX)
	{
		m_events.addEvent(sliderNotes, new MidiFile::MidiChunk_Track::SysexEvent(0xFF, 4, 0));
		sliderNotes = UINT32_MAX;
	}
}

template<>
void MidiTrackWriter::insertNoteEvents(const NodeTrack<DrumNote>& track)
{
	for (const auto& vec : track.m_difficulties[0].m_events)
		for (const auto& ev : vec.second)
			m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

	for (const auto& vec : track.m_difficulties[0].m_effects)
		for (const auto& effect : vec.second)
			if (effect->getMidiNote() != -1)
			{
				m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
				m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
			}
			else
				for (int lane = 120; lane < 125; ++lane)
				{
					m_events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane));
					m_events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane, 0));
				}

	if (!track.hasNotes())
		return;

	m_doWrite = true;

	bool useDynamics = false;
	auto processNote = [&](const std::pair<uint32_t, DrumNote>& note,
		char baseMidiNote,
		const std::pair<uint32_t, DrumNote>* const prev)
	{
		auto placeNote = [&](char midiNote, char velocity)
		{
			m_events.addEvent(note.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, velocity));
			m_events.addEvent(note.first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, midiNote, 0));
		};

		if (note.second.m_open)
		{
			if (note.second.m_open.m_isDoubleBass)
				placeNote(95, 100);
			else
				placeNote(baseMidiNote, 100);
		}

		for (char col = 0; col < 4; ++col)
			if (note.second.m_colors[col])
			{
				if (note.second.m_colors[col].m_isAccented)
				{
					// For now, use 127 as the default accent velocity
					placeNote(baseMidiNote + col + 1, 127);
					useDynamics = true;
				}
				else if (note.second.m_colors[col].m_isGhosted)
				{
					// For now, use 1 as the default accent velocity
					placeNote(baseMidiNote + col + 1, 1);
					useDynamics = true;
				}
				else
					placeNote(baseMidiNote + col + 1, 100);
			}

		if (note.second.m_fifthLane)
		{
			if (note.second.m_fifthLane.m_isAccented)
			{
				// For now, use 127 as the default accent velocity
				placeNote(baseMidiNote + 5, 127);
				useDynamics = true;
			}
			else if (note.second.m_fifthLane.m_isGhosted)
			{
				// For now, use 1 as the default accent velocity
				placeNote(baseMidiNote + 5, 1);
				useDynamics = true;
			}
			else
				placeNote(baseMidiNote + 5, 100);
		}
	};

	auto expertIter = track.m_difficulties[0].m_notes.begin();
	auto hardIter = track.m_difficulties[1].m_notes.begin();
	auto mediumIter = track.m_difficulties[2].m_notes.begin();
	auto easyIter = track.m_difficulties[3].m_notes.begin();
	bool expertValid = expertIter != track.m_difficulties[0].m_notes.end();
	bool hardValid = hardIter != track.m_difficulties[1].m_notes.end();
	bool mediumValid = mediumIter != track.m_difficulties[2].m_notes.end();
	bool easyValid = easyIter != track.m_difficulties[3].m_notes.end();

	uint32_t toms[3] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };

	int adjustWithDifficulty = 0;
	if (!expertValid)
		++adjustWithDifficulty;
	if (!hardValid)
		++adjustWithDifficulty;
	if (!mediumValid)
		++adjustWithDifficulty;

	auto adjustToms = [&](const std::pair<uint32_t, DrumNote>& pair)
	{
		for (int lane = 0; lane < 3; ++lane)
		{
			if (pair.second.m_colors[1 + lane])
			{
				if (pair.second.m_colors[1 + lane].m_isCymbal)
				{
					// Ensure that toms are disabled
					if (toms[lane] != UINT32_MAX)
					{
						// Create Note off Event for the tom marker
						m_events.addEvent(toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
						toms[lane] = UINT32_MAX;
					}
				}
				else
				{
					// Ensure toms are enabled
					if (toms[lane] == UINT32_MAX)
						// Create Note On Event for the tom marker
						m_events.addEvent(pair.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane));
					toms[lane] = pair.first + 1;
				}
			}
		}
	};

	while (expertValid || hardValid || mediumValid || easyValid)
	{
		while (expertValid &&
			(!hardValid || expertIter->first <= hardIter->first) &&
			(!mediumValid || expertIter->first <= mediumIter->first) &&
			(!easyValid || expertIter->first <= easyIter->first))
		{
			if (adjustWithDifficulty == 0)
				adjustToms(*expertIter);

			if (expertIter != track.m_difficulties[0].m_notes.begin())
				processNote(*expertIter, 96, (expertIter - 1)._Ptr);
			else
				processNote(*expertIter, 96, nullptr);

			if (adjustWithDifficulty == 0 && expertIter->second.m_isFlamed)
			{
				m_events.addEvent(expertIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
				m_events.addEvent(expertIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
			}

			expertValid = ++expertIter != track.m_difficulties[0].m_notes.end();
		}

		while (hardValid &&
			(!expertValid || hardIter->first < expertIter->first) &&
			(!mediumValid || hardIter->first <= mediumIter->first) &&
			(!easyValid || hardIter->first <= easyIter->first))
		{
			if (adjustWithDifficulty == 1)
				adjustToms(*hardIter);

			if (hardIter != track.m_difficulties[1].m_notes.begin())
				processNote(*hardIter, 84, (hardIter - 1)._Ptr);
			else
				processNote(*hardIter, 84, nullptr);

			if (adjustWithDifficulty == 1 && hardIter->second.m_isFlamed)
			{
				m_events.addEvent(hardIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
				m_events.addEvent(hardIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
			}

			hardValid = ++hardIter != track.m_difficulties[1].m_notes.end();
		}

		while (mediumValid &&
			(!expertValid || mediumIter->first < expertIter->first) &&
			(!hardValid || mediumIter->first < hardIter->first) &&
			(!easyValid || mediumIter->first <= easyIter->first))
		{
			if (adjustWithDifficulty == 2)
				adjustToms(*mediumIter);

			if (mediumIter != track.m_difficulties[2].m_notes.begin())
				processNote(*mediumIter, 72, (mediumIter - 1)._Ptr);
			else
				processNote(*mediumIter, 72, nullptr);

			if (adjustWithDifficulty == 2 && mediumIter->second.m_isFlamed)
			{
				m_events.addEvent(mediumIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
				m_events.addEvent(mediumIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
			}
			mediumValid = ++mediumIter != track.m_difficulties[2].m_notes.end();
		}

		while (easyValid &&
			(!expertValid || easyIter->first < expertIter->first) &&
			(!hardValid || easyIter->first < hardIter->first) &&
			(!mediumValid || easyIter->first < mediumIter->first))
		{
			if (adjustWithDifficulty == 3)
				adjustToms(*easyIter);

			if (easyIter != track.m_difficulties[3].m_notes.begin())
				processNote(*easyIter, 60, (easyIter - 1)._Ptr);
			else
				processNote(*easyIter, 60, nullptr);

			if (adjustWithDifficulty == 3 && easyIter->second.m_isFlamed)
			{
				m_events.addEvent(easyIter->first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109));
				m_events.addEvent(easyIter->first + 1, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 109, 0));
			}
			easyValid = ++easyIter != track.m_difficulties[3].m_notes.end();
		}
	}

	// Disable any active tom markers
	for (int lane = 0; lane < 3; ++lane)
		if (toms[lane] != UINT32_MAX)
		{
			// Create Note off Event for the tom marker
			m_events.addEvent(toms[lane], new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, 110 + lane, 0));
			toms[lane] = UINT32_MAX;
		}

	if (useDynamics)
		m_events.addEvent(0, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, "[ENABLE_CHART_DYNAMICS]"));
}
