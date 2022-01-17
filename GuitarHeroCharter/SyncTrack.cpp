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

void SyncTrack::writeSync(const uint32_t position, std::ofstream& outFile, const SyncTrack* prev) const
{
	if (!prev || prev->m_timeSigNumerator != m_timeSigNumerator || prev->m_timeSigDenomExponent != m_timeSigDenomExponent)
	{
		outFile << "  " << position << " = TS " << m_timeSigNumerator;
		if (m_timeSigDenomExponent != 2)
			outFile << m_timeSigDenomExponent;
		outFile << '\n';
	}

	if (!prev || prev->m_bpm != m_bpm)
		outFile << "  " << position << " = B " << m_bpm * 1000 << '\n';
}

bool SyncTrack::writeEvents(const uint32_t initial, const uint32_t endPosition, std::ofstream& outFile) const
{
	if (m_globalEvents.size())
	{
		auto iter = m_globalEvents.lower_bound(initial);
		for (; iter != m_globalEvents.end() && iter->first < endPosition; ++iter)
		{
			for (const auto& str : iter->second)
				outFile << "  " << iter->first << " = E \"" << str << "\"\n";
		}
		return iter != m_globalEvents.end();
	}
	return false;
}

void SyncTrack::writeNoteTracks_chart(Instrument track, int difficulty, std::ofstream& outFile) const
{
	switch (track)
	{
	case Instrument::Guitar_lead:
		m_leadGuitar.write_chart(difficulty, outFile);
		break;
	case Instrument::Guitar_lead_6:
		m_leadGuitar_6.write_chart(difficulty, outFile);
		break;
	case Instrument::Guitar_bass:
		m_bassGuitar.write_chart(difficulty, outFile);
		break;
	case Instrument::Guitar_bass_6:
		m_bassGuitar_6.write_chart(difficulty, outFile);
		break;
	case Instrument::Guitar_rhythm:
		m_rhythmGuitar.write_chart(difficulty, outFile);
		break;
	case Instrument::Guitar_coop:
		m_coopGuitar.write_chart(difficulty, outFile);
		break;
	case Instrument::Drums:
		m_drums.write_chart(difficulty, outFile);
		break;
	case Instrument::Drums_5:
		m_drums_5Lane.write_chart(difficulty, outFile);
	}
}

bool SyncTrack::hasNotes(Instrument track, int diff) const
{
	switch (track)
	{
	case Instrument::Guitar_lead:
		return m_leadGuitar.hasNotes(diff);
	case Instrument::Guitar_lead_6:
		return m_leadGuitar_6.hasNotes(diff);
	case Instrument::Guitar_bass:
		return m_bassGuitar.hasNotes(diff);
	case Instrument::Guitar_bass_6:
		return m_bassGuitar_6.hasNotes(diff);
	case Instrument::Guitar_rhythm:
		return m_rhythmGuitar.hasNotes(diff);
	case Instrument::Guitar_coop:
		return m_coopGuitar.hasNotes(diff);
	case Instrument::Drums:
		return m_drums.hasNotes(diff);
	case Instrument::Drums_5:
		return m_drums_5Lane.hasNotes(diff);
	default:
		return false;
	}
}
