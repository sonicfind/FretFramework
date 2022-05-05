#pragma once
#include "Phrases/Phrases.h"
#include "../VectorIteration.h"

template <typename T>
class BasicTrack;

template <typename T>
class Difficulty
{
	friend class BasicTrack<T>;

	const char* const m_name;
	std::vector<std::pair<uint32_t, T>> m_notes;
	std::vector<std::pair<uint32_t, std::vector<SustainablePhrase*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_events;

public:
	void load_chart_V1(std::fstream& inFile);
	void load_cht(std::fstream& inFile);
	void save_cht(std::fstream& outFile) const;

	void clear()
	{
		m_notes.clear();
		m_events.clear();
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
		m_effects.clear();
	}

private:
	Difficulty(const char* name)
		: m_name(name) {}

	void addNote(uint32_t position, int note, uint32_t sustain = 0)
	{
		VectorIteration::try_emplace(m_notes, position).init(note, sustain);
	}

	void addNoteFromMid(uint32_t position, int note, size_t endOffset, uint32_t sustain = 0)
	{
		if (sustain < 20)
			sustain = 0;

		size_t index = m_notes.size() - endOffset;
		while (index < m_notes.size() - 1 && m_notes[index].first < position)
			++index;

		m_notes[index].second.init(note, sustain);
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

	bool modifyNote(uint32_t position, char modifier, bool toggle = true)
	{
		try
		{
			return VectorIteration::getIterator(m_notes, position).modify(modifier, toggle);
		}
		catch (...)
		{
			return false;
		}
	}

	bool modifyColor(uint32_t position, int note, char modifier)
	{
		try
		{
			return VectorIteration::getIterator(m_notes, position).modifyColor(note, modifier);
		}
		catch (...)
		{
			return false;
		}
	}

	uint32_t getNumActiveColors(uint32_t position)
	{
		try
		{
			return VectorIteration::getIterator(m_notes, position).getNumActiveColors();
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

	~Difficulty()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}
};
