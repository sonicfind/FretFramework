#pragma once
#include "Difficulty.h"
#include "Drums/DrumNote_bch.hpp"
#include "Drums/DrumNote_cht.hpp"

template <>
class Difficulty_Scan<DrumNote_Legacy>
{
	friend class InstrumentalTrack_Scan<DrumNote_Legacy>;
	bool m_isFiveLane = false;

public:
	bool isFiveLane() const { return m_isFiveLane; };
	bool scan_chart_V1(TextTraversal& traversal);

private:
	Difficulty_Scan() = default;
	void init_chart_V1(unsigned char lane, uint32_t sustain)
	{
		DrumNote_Legacy note;
		note.init_chartV1(lane, sustain);
		if (note.m_colors[4])
			m_isFiveLane = true;
	}
};

template <>
class Difficulty<DrumNote_Legacy>
{
	friend class InstrumentalTrack<DrumNote_Legacy>;
	friend class Difficulty<DrumNote<4, DrumPad_Pro>>;
	friend class Difficulty<DrumNote<5, DrumPad>>;

	std::vector<std::pair<uint32_t, DrumNote_Legacy>> m_notes;
	std::vector<std::pair<uint32_t, std::vector<SustainablePhrase*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::u32string>>> m_events;

	bool m_isFiveLane = false;

public:
	bool isFiveLane() const { return m_isFiveLane; };
	void load_chart_V1(TextTraversal& traversal);

	void clear()
	{
		m_notes.clear();
		m_events.clear();
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
		m_effects.clear();
		m_isFiveLane = false;
	}

private:
	void init_chart_V1(unsigned char lane, uint32_t sustain)
	{
		DrumNote_Legacy& note = m_notes.back().second;
		note.init_chartV1(lane, sustain);
		if (note.m_colors[4])
			m_isFiveLane = true;
	}

	void setColor_linear(uint32_t position, int lane, uint32_t sustain = 0)
	{
		if (sustain < 20)
			sustain = 0;

		auto iter = m_notes.end() - 1;
		while (iter->first != position)
			--iter;

		DrumNote_Legacy& note = iter->second;
		note.init(lane, sustain);
		if (note.m_colors[4])
			m_isFiveLane = true;
	}

	void addPhrase(uint32_t position, SustainablePhrase* effect)
	{
		VectorIteration::try_emplace(m_effects, position).push_back(effect);
	}

	// Returns whether this difficulty contains notes, effects, soloes, or other events
	bool occupied() const { return !m_notes.empty() || !m_events.empty() || !m_effects.empty(); }

	~Difficulty()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}
};

template <>
template <>
Difficulty<DrumNote<4, DrumPad_Pro>>& Difficulty<DrumNote<4, DrumPad_Pro>>::operator=(const Difficulty<DrumNote_Legacy>& diff);

template <>
template <>
Difficulty<DrumNote<5, DrumPad>>& Difficulty<DrumNote<5, DrumPad>>::operator=(const Difficulty<DrumNote_Legacy>& diff);
