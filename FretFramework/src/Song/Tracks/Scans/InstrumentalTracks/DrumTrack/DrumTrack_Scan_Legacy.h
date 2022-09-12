#pragma once
#include "../InstrumentalTrack_Scan.h"
#include "Drums/DrumNote_bch.hpp"
#include "Drums/DrumNote_cht.hpp"

template <>
class InstrumentalTrack_Scan<DrumNote_Legacy>
{
private:
	int m_scanValue = 0;

	enum DrumType
	{
		UNKNOWN,
		FOURLANE_PRO,
		FIVELANE,
	} m_drumType = UNKNOWN;

public:
	DrumType getDrumType() const
	{
		return m_drumType;
	}

	bool isFiveLane() const
	{
		return m_drumType == FIVELANE;
	}

	bool isDrum4Pro() const
	{
		return m_drumType == FOURLANE_PRO;
	}

	int getValue() const { return m_scanValue; }
	void scan_chart_V1(int diff, TextTraversal& traversal);
	void scan_midi(MidiTraversal& traversal);
};
