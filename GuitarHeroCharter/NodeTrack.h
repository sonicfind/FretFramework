#pragma once
#include "TimedNode.h"
#include <map>
#include <vector>
#include <string>
enum class DifficultyLevel
{
	Expert,
	Hard,
	Medium,
	Easy
};

template <class T>
class NodeTrack
{
	struct Difficulty
	{
		std::map<uint32_t, T> m_notes;
		std::map<uint32_t, Fret> m_starPower;
	};
	Difficulty m_difficulties[4];
	std::map<uint32_t, std::vector<std::string>> m_localEvents;
public:
	NodeTrack() = default;

	void fromChart(std::stringstream& ss, DifficultyLevel difficulty, uint32_t position)
	{
		char type;
		ss >> type;

		if (type == 'E')
		{
			std::string line;
			std::getline(ss, line);
			m_localEvents[position].push_back(std::move(line));
		}
		else
		{
			size_t lane;
			uint32_t sustain;
			ss >> lane >> sustain;

			if (type == 'S')
				m_difficulties[static_cast<int>(difficulty)].m_starPower[position].init(sustain);
			else
				m_difficulties[static_cast<int>(difficulty)].m_notes[position].init_chart(lane, sustain);
		}
	}
};
