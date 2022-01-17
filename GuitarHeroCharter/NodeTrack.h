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
		std::map<uint32_t, std::vector<std::string>> m_events;
		void read_chart(std::stringstream& ss, const uint32_t position)
		{
			char type;
			ss >> type;

			if (type == 'E')
			{
				std::string line;
				std::getline(ss, line);
				m_events[position].push_back(std::move(line));
			}
			else
			{
				size_t lane;
				uint32_t sustain;
				ss >> lane >> sustain;

				if (type == 'S')
					m_starPower[position].init(sustain);
				else
					m_notes[position].init_chart(lane, sustain);
			}
		}

		operator bool() const { return m_notes.size() || m_starPower.size() || m_events.size(); }
	};
	Difficulty m_difficulties[4];
	
public:
	NodeTrack() = default;

	void read_chart(uint32_t position, DifficultyLevel difficulty, std::stringstream& ss)
	{
		m_difficulties[static_cast<int>(difficulty)].read_chart(ss, position);
	}
};
