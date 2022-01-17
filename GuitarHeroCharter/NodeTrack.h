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

		void write_chart(std::ofstream& outFile) const
		{
			auto noteIter = m_notes.begin();
			auto starIter = m_starPower.begin();
			auto eventIter = m_events.begin();
			bool notesValid = noteIter != m_notes.end();
			bool starValid = starIter != m_starPower.end();
			bool eventValid = eventIter != m_events.end();

			while (notesValid || starValid || eventValid)
			{
				while (notesValid &&
					(!eventValid || noteIter->first <= eventIter->first) &&
					(!starValid || noteIter->first <= starIter->first))
				{
					noteIter->second.write_chart(noteIter->first, outFile);
					notesValid = ++noteIter != m_notes.end();
				}

				while (starValid &&
					(!notesValid || starIter->first < noteIter->first) &&
					(!eventValid || starIter->first <= eventIter->first))
				{
					starIter->second.write_chart(starIter->first, 2, outFile, 'S');
					starValid = ++starIter != m_starPower.end();
				}

				while (eventValid &&
					(!notesValid || eventIter->first < noteIter->first) &&
					(!starValid || eventIter->first < starIter->first))
				{
					for (const auto& str : eventIter->second)
						outFile << "  " << eventIter->first << " = \"" << str << "\"\n";
					eventValid = ++eventIter != m_events.end();
				}
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

	void write_chart(int diff, std::ofstream& outFile) const
	{
		m_difficulties[diff].write_chart(outFile);
	}

	bool hasNotes(int diff) const
	{
		return m_difficulties[diff];
	}
};
