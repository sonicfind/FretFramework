#pragma once
#include "NodeTrack.h"
enum class Instrument
{
	Guitar_lead,
	Guitar_lead_6,
	Guitar_bass,
	Guitar_bass_6,
	Guitar_rhythm,
	Guitar_coop,
	Drums,
	Drums_5,
	Vocals,
	Keys,
	None
};

class SyncTrack
{
	float m_bpm = 120;
	uint32_t m_timeSigNumerator = 4;
	uint32_t m_timeSigDenomExponent = 2;

	NodeTrack<GuitarNote_5Fret> m_leadGuitar;
	NodeTrack<GuitarNote_6Fret> m_leadGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_bassGuitar;
	NodeTrack<GuitarNote_6Fret> m_bassGuitar_6;
	NodeTrack<GuitarNote_5Fret> m_rhythmGuitar;
	NodeTrack<GuitarNote_5Fret> m_coopGuitar;
	NodeTrack<DrumNote<4, DrumPad_Pro>> m_drums;
	NodeTrack<DrumNote<5, DrumPad>> m_drums_5Lane;
	std::map<uint32_t, std::vector<std::string>> m_globalEvents;

public:
	SyncTrack() = default;
	bool setSyncValues(std::stringstream& ss);
	void addEvent(const uint32_t position, const std::string& ev);
	void readNote(uint32_t position, Instrument track, DifficultyLevel difficulty, std::stringstream& ss);
};