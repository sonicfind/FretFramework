#pragma once
#include "../InstrumentalTrack.h"
#include "Drums/DrumNote.h"

class DrumTrackConverter
{
public:
	template <class DrumNoteType>
	static void convert(InstrumentalTrack<DrumNote_Legacy>& legacy, InstrumentalTrack<DrumNoteType>* track)
	{
		std::pair<uint32_t, DrumNoteType> drumPair;
		for (int i = 0; i < 5; ++i)
			if (legacy.m_difficulties[i].occupied())
			{
				track->m_difficulties[i].m_effects = std::move(legacy.m_difficulties[i].m_effects);
				track->m_difficulties[i].m_events = std::move(legacy.m_difficulties[i].m_events);
				track->m_difficulties[i].m_notes.reserve(legacy.m_difficulties[i].m_notes.size());
				track->m_difficulties[i].m_notes.clear();
				for (const auto& pair : legacy.m_difficulties[i].m_notes)
				{
					drumPair.first = pair.first;
					pair.second.convert(drumPair.second);
					track->m_difficulties[i].m_notes.push_back(drumPair);
				}
			}
	}
};
