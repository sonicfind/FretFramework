#pragma once
#include "Song/Phrases/Phrases.h"
#include "VectorIteration.h"
#include "FileTraversal/TextFileTraversal.h"
#include "FileTraversal/BCHFileTraversal.h"

template <typename T>
class InstrumentalTrack;

template <typename T>
class Difficulty
{
	friend class InstrumentalTrack<T>;

	const char* const m_name;
	const char m_difficultyID;
	std::vector<std::pair<uint32_t, T>> m_notes;
	std::vector<std::pair<uint32_t, std::vector<SustainablePhrase*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::u32string>>> m_events;

public:
	template <typename U>
	Difficulty& operator=(Difficulty<U>&& diff) { return *this; }

	void load_chart_V1(TextTraversal& traversal);
	void load_cht(TextTraversal& traversal);
	void save_cht(std::fstream& outFile) const;

	void load_bch(BCHTraversal& traversal);
	void save_bch(std::fstream& outFile) const;

	void clear()
	{
		m_notes.clear();
		m_events.clear();
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
		m_effects.clear();
	}

	void adjustTicks(float multiplier)
	{
		for (std::pair<uint32_t, T>& note : m_notes)
		{
			note.first = uint32_t(note.first * multiplier);
			note.second *= multiplier;
		}

		for (auto& vec : m_effects)
		{
			vec.first = uint32_t(vec.first * multiplier);
			for (SustainablePhrase* eff : vec.second)
				*eff *= multiplier;
		}

		for (auto& ev : m_events)
			ev.first = uint32_t(ev.first * multiplier);
	}

private:
	constexpr Difficulty(const char* name, char difficultyID)
		: m_name(name)
		, m_difficultyID(difficultyID) {}

	~Difficulty()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}

	void init_chart_V1(unsigned char lane, uint32_t sustain)
	{
		m_notes.back().second.init_chartV1(lane, sustain);
	}

	void init_single(TextTraversal& traversal)
	{
		m_notes.back().second.init_single(traversal);
	}

	void init_single(BCHTraversal& traversal)
	{
		m_notes.back().second.init_single(traversal);
	}

	void init_chord(TextTraversal& traversal)
	{
		m_notes.back().second.init_chord(traversal);
	}

	void init_chord(BCHTraversal& traversal)
	{
		m_notes.back().second.init_chord(traversal);
	}

	void addNote(uint32_t position, int note, uint32_t sustain = 0)
	{
		VectorIteration::try_emplace(m_notes, position).init(note, sustain);
	}

	void setColor_linear(uint32_t position, int note, uint32_t sustain = 0)
	{
		if (sustain < 20)
			sustain = 0;

		auto iter = m_notes.end() - 1;
		while (iter->first != position)
			--iter;

		iter->second.init(note, sustain);
	}

	void addPhrase(uint32_t position, SustainablePhrase* effect)
	{
		VectorIteration::try_emplace(m_effects, position).push_back(effect);
	}

	void addEvent(uint32_t position, const std::string& ev)
	{
		if (ev[0] != '\"')
			VectorIteration::try_emplace(m_events, position).push_back(ev);
		else
			VectorIteration::try_emplace(m_events, position).push_back(ev.substr(1, ev.length() - 2));
	}

	void modifyNote(uint32_t position, char modifier, int lane = 0)
	{
		try
		{
			VectorIteration::getElement(m_notes, position).modify(modifier, lane);
		}
		catch (...) {}
	}

	uint32_t getNumActive(uint32_t position)
	{
		try
		{
			return VectorIteration::getElement(m_notes, position).getNumActive();
		}
		catch (std::out_of_range oor)
		{
			return 0;
		}
	}

	// Returns whether this difficulty contains notes
	// ONLY checks for notes
	bool hasNotes() const { return m_notes.size(); }

	// Returns whether this difficulty contains notes, effects, soloes, or other events
	bool occupied() const { return !m_notes.empty() || !m_events.empty() || !m_effects.empty(); }
};

