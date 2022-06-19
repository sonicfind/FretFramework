#pragma once
#include "../InstrumentalTrack.h"
#include "../Difficulty/Difficulty_DrumLegacy.h"

template <>
class InstrumentalTrack_Scan<DrumNote_Legacy>
{
private:
	int m_scanValaue = 0;
	Difficulty_Scan<DrumNote_Legacy> m_difficulties[5];

	bool m_isFiveLane = false;

public:
	bool isFiveLane() const
	{
		if (m_isFiveLane)
			return true;

		for (auto& diff : m_difficulties)
			if (diff.isFiveLane())
				return true;
		return false;
	}

	int getValue() const { return m_scanValaue; }
	void scan_chart_V1(int diff, TextTraversal& traversal);
	void scan_midi(MidiTraversal& traversal);
};

template <>
class InstrumentalTrack<DrumNote_Legacy>
{
	friend class InstrumentalTrack<DrumNote<4, DrumPad_Pro>>;
	friend class InstrumentalTrack<DrumNote<5, DrumPad>>;

	Difficulty<DrumNote_Legacy> m_difficulties[5];

	bool m_isFiveLane = false;

public:
	static unsigned char s_starPowerReadNote;
	bool isFiveLane() const
	{
		if (m_isFiveLane)
			return true;

		for (auto& diff : m_difficulties)
			if (diff.isFiveLane())
				return true;
		return false;
	}

	void scan_chart_V1(int diff, TextTraversal& traversal, NoteTrack_Scan*& track) const;
	void load_chart_V1(int diff, TextTraversal& traversal);

	void load_midi(MidiTraversal& traversal);

	// Returns whether any difficulty in this track contains notes, effects, soloes, or other events
	bool occupied() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.occupied())
				return true;
		return false;
	}
};

template <>
template <>
InstrumentalTrack<DrumNote<4, DrumPad_Pro>>& InstrumentalTrack<DrumNote<4, DrumPad_Pro>>::operator=(const InstrumentalTrack<DrumNote_Legacy>& track);

template <>
template <>
InstrumentalTrack<DrumNote<5, DrumPad>>& InstrumentalTrack<DrumNote<5, DrumPad>>::operator=(const InstrumentalTrack<DrumNote_Legacy>& track);
