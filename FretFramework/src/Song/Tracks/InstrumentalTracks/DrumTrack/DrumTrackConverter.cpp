#include "DrumTrackConverter.h"

std::pair<uint32_t, DrumNote<4, DrumPad_Pro>> DrumTrackConverter::s_4LanePair;
std::pair<uint32_t, DrumNote<5, DrumPad>> DrumTrackConverter::s_5LanePair;
void DrumTrackConverter::convert(InstrumentalTrack<DrumNote_Legacy>& legacy, InstrumentalTrack<DrumNote<4, DrumPad_Pro>>* track)
{
	for (int i = 0; i < 5; ++i)
		if (legacy.m_difficulties[i].occupied())
		{
			track->m_difficulties[i].m_effects = std::move(legacy.m_difficulties[i].m_effects);
			track->m_difficulties[i].m_events = std::move(legacy.m_difficulties[i].m_events);
			track->m_difficulties[i].m_notes.reserve(legacy.m_difficulties[i].m_notes.size());
			for (const auto& pair : legacy.m_difficulties[i].m_notes)
			{
				s_4LanePair.first = pair.first;
				pair.second.convert(s_4LanePair.second);
				track->m_difficulties[i].m_notes.push_back(s_4LanePair);
			}
		}
}

void DrumTrackConverter::convert(InstrumentalTrack<DrumNote_Legacy>& legacy, InstrumentalTrack<DrumNote<5, DrumPad>>* track)
{
	for (int i = 0; i < 5; ++i)
		if (legacy.m_difficulties[i].occupied())
		{
			track->m_difficulties[i].m_effects = std::move(legacy.m_difficulties[i].m_effects);
			track->m_difficulties[i].m_events = std::move(legacy.m_difficulties[i].m_events);
			track->m_difficulties[i].m_notes.reserve(legacy.m_difficulties[i].m_notes.size());
			for (const auto& pair : legacy.m_difficulties[i].m_notes)
			{
				s_4LanePair.first = pair.first;
				pair.second.convert(s_5LanePair.second);
				track->m_difficulties[i].m_notes.push_back(s_5LanePair);
			}
		}
}
