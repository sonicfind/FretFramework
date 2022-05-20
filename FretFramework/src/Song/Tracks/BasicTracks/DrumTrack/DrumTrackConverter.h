#pragma once
#include "../InstrumentalTrack.h"
#include "Drums/DrumNote.h"

class DrumTrackConverter
{
	static std::pair<uint32_t, DrumNote<4, DrumPad_Pro>> s_4LanePair;
	static std::pair<uint32_t, DrumNote<5, DrumPad>> s_5LanePair;
public:
	static void convert(InstrumentalTrack<DrumNote_Legacy>& legacy, InstrumentalTrack<DrumNote<4, DrumPad_Pro>>* track);
	static void convert(InstrumentalTrack<DrumNote_Legacy>& legacy, InstrumentalTrack<DrumNote<5, DrumPad>>* track);
};
