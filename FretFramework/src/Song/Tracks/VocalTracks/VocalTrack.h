#pragma once
#include <iostream>
#include "Vocals/Vocal.h"
#include "Vocals/VocalPercussion.h"
#include "Phrases/Phrases.h"
#include "Midi/MidiFile.h"
#include "../VectorIteration.h"
#include "../TextFileManip.h"
#include "NoteExceptions.h"
using namespace MidiFile;

template <size_t numTracks>
class VocalTrack
{
	const char* const m_name;
	std::vector<std::pair<uint32_t, Vocal>> m_vocals[numTracks];
	std::vector<std::pair<uint32_t, VocalPercussion>> m_percussion;
	std::vector<std::pair<uint32_t, std::vector<Phrase*>>> m_effects;
	std::vector<std::pair<uint32_t, std::vector<std::string>>> m_events;

	void init_cht_single(uint32_t position, TextTraversal& traversal);
	void init_cht_chord(uint32_t position, TextTraversal& traversal);
	void modify_cht(uint32_t position, TextTraversal& traversal);
	void vocalize_cht(uint32_t position, TextTraversal& traversal);

	uint32_t getLongestSustain(uint32_t position) const
	{
		// If percussion is the only note at this position, then sustain should end up as 0
		uint32_t sustain = 0;
		for (const auto& track : m_vocals)
		{
			if (!track.empty())
			{
				try
				{
					const Vocal& vocal = VectorIteration::getIterator(track, position);
					if (vocal.getSustain() > sustain)
						sustain = vocal.getSustain();
				}
				catch (...) {}
			}
		}
		return sustain;
	}

public:
	VocalTrack(const char* name) : m_name(name) {}

	// Returns whether this track contains any notes
	// ONLY checks for notes
	bool hasNotes() const
	{
		for (const auto& track : m_vocals)
			if (track.size())
				return true;

		return m_percussion.size();
	}

	// Returns whether this track contains any notes, effects, soloes, or other events
	bool occupied() const
	{
		return hasNotes() ||
			m_effects.size() ||
			m_events.size();
	}

	void addLyric(int lane, uint32_t position, const std::string& lyric)
	{
		VectorIteration::try_emplace(m_vocals[lane], position).setLyric(lyric);
	}

	void addPercussion(uint32_t position, bool playable)
	{
		VocalPercussion& perc = VectorIteration::try_emplace(m_percussion, position);
		if (!playable)
			perc.modify('N');
	}

	void addPhrase(uint32_t position, Phrase* effect)
	{
		VectorIteration::try_emplace(m_effects, position).push_back(effect);
	}

	void addEvent(uint32_t position, const std::string& ev)
	{
		VectorIteration::try_emplace(m_events, position).push_back(ev);
	}

	void setPitch(int lane, uint32_t position, char pitch, uint32_t sustain = 0)
	{
		try
		{
			VectorIteration::getIterator(m_vocals[lane], position).setPitch(pitch);
		}
		catch (...)
		{
			return false;
		}
	}

	void clear()
	{
		for (auto& track : m_vocals)
			track.clear();
		m_percussion.clear();
		m_events.clear();
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
		m_effects.clear();
	}

	void load_cht(TextTraversal& traversal);
	void save_cht(std::fstream& outFile) const;

	void load_midi(int index, const unsigned char* currPtr, const unsigned char* const end);
protected:
	void save_midi(const std::string& name, int trackIndex, std::fstream& outFile) const;
public:
	int save_midi(std::fstream& outFile) const;
	void adjustTicks(float multiplier)
	{
		for (auto& track : m_vocals)
		{
			for (std::pair<uint32_t, Vocal>& vocal : track)
			{
				vocal.first = uint32_t(vocal.first * multiplier);
				vocal.second *= multiplier;
			}
		}

		for (auto& perc : m_percussion)
			perc.first = uint32_t(perc.first * multiplier);

		for (auto& vec : m_effects)
		{
			vec.first = uint32_t(vec.first * multiplier);
			for (Phrase* eff : vec.second)
				*eff *= multiplier;
		}

		for (auto& ev : m_events)
			ev.first = uint32_t(ev.first * multiplier);
	}

	~VocalTrack()
	{
		for (auto& vec : m_effects)
			for (auto& eff : vec.second)
				delete eff;
	}
};

template<>
void VocalTrack<1>::save_cht(std::fstream& outFile) const;

template<>
void VocalTrack<3>::save_cht(std::fstream& outFile) const;
