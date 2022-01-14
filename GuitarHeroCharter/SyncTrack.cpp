#include "SyncTrack.h"
bool SyncTrack::setSyncValues(std::stringstream& ss)
{
	std::string type;
	ss >> type;
	if (type[0] == 'T')
	{
		ss >> m_timeSigNumerator;
		ss >> m_timeSigDenomExponent;
	}
	else if (type[0] == 'B')
	{
		ss >> m_bpm;
		m_bpm *= .1f;
	}
	else
		return false;
	return true;
}

void SyncTrack::addEvent(uint32_t position, const std::string& ev)
{
	m_globalEvents[position].push_back(std::move(ev));
}

void SyncTrack::readNote(std::stringstream& ss, Instrument track, DifficultyLevel diff, uint32_t position)
{
	switch (track)
	{
	case Instrument::Guitar_lead:
		m_leadGuitar.fromChart(ss, diff, position);
		break;
	case Instrument::Guitar_lead_6:
		m_leadGuitar_6.fromChart(ss, diff, position);
		break;
	case Instrument::Guitar_bass:
		m_bassGuitar.fromChart(ss, diff, position);
		break;
	case Instrument::Guitar_bass_6:
		m_bassGuitar_6.fromChart(ss, diff, position);
		break;
	case Instrument::Guitar_rhythm:
		m_rhythmGuitar.fromChart(ss, diff, position);
		break;
	case Instrument::Guitar_coop:
		m_coopGuitar.fromChart(ss, diff, position);
		break;
	case Instrument::Drums:
		m_drums.fromChart(ss, diff, position);
		break;
	case Instrument::Drums_5:
		m_drums_5Lane.fromChart(ss, diff, position);
	}
}
