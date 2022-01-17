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
		m_bpm *= .001f;
	}
	else
		return false;
	return true;
}

void SyncTrack::addEvent(const uint32_t position, const std::string& ev)
{
	m_globalEvents[position].push_back(ev);
}

void SyncTrack::readNote(uint32_t position, Instrument track, DifficultyLevel diff, std::stringstream& ss)
{
	switch (track)
	{
	case Instrument::Guitar_lead:
		m_leadGuitar.read_chart(position, diff, ss);
		break;
	case Instrument::Guitar_lead_6:
		m_leadGuitar_6.read_chart(position, diff, ss);
		break;
	case Instrument::Guitar_bass:
		m_bassGuitar.read_chart(position, diff, ss);
		break;
	case Instrument::Guitar_bass_6:
		m_bassGuitar_6.read_chart(position, diff, ss);
		break;
	case Instrument::Guitar_rhythm:
		m_rhythmGuitar.read_chart(position, diff, ss);
		break;
	case Instrument::Guitar_coop:
		m_coopGuitar.read_chart(position, diff, ss);
		break;
	case Instrument::Drums:
		m_drums.read_chart(position, diff, ss);
		break;
	case Instrument::Drums_5:
		m_drums_5Lane.read_chart(position, diff, ss);
	}
}
