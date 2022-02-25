#pragma once
#include "TimedNode.h"
#include <map>
#include <vector>
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
		
		void read_chart(std::fstream& inFile, const bool version2)
		{
			std::string line;
			while (std::getline(inFile, line) && line.find('}') == std::string::npos)
			{
				std::stringstream ss(line);
				uint32_t position;
				ss >> position;
				ss.ignore(5, '=');

				char type;
				ss >> type;
				
				if (type == 'E')
				{
					std::string line;
					ss.ignore(1);
					std::getline(ss, line);
					m_events[position].push_back(std::move(line));
				}
				else if (type == 'M')
					m_notes[position].init_chart2_modifier(ss);
				else
				{
					size_t lane;
					uint32_t sustain;
					ss >> lane >> sustain;

					if (type == 'S')
						m_starPower[position].init(sustain);
					else if (!version2)
						m_notes[position].init_chart(lane, sustain);
					else
						m_notes[position].init_chart2(lane, sustain);
				}
			}
		}

		void write_chart(std::fstream& outFile) const
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
						outFile << "  " << eventIter->first << " = E " << str << '\n';
					eventValid = ++eventIter != m_events.end();
				}
			}
		}

		operator bool() const { return m_notes.size() || m_starPower.size() || m_events.size(); }
	};
	Difficulty m_difficulties[4];
	
public:
	Difficulty& operator[](int i)
	{
		return m_difficulties[i];
	}

	const Difficulty& operator[](int i) const
	{
		return m_difficulties[i];
	}

	void write_chart(const std::string_view& ins, std::fstream& outFile) const
	{
		static std::string_view difficultyStrings[] = { "Expert", "Hard", "Medium", "Easy" };
		for (int diff = 0; diff < 4; ++diff)
		{
			if (m_difficulties[diff])
			{
				outFile << "[" << difficultyStrings[diff] << ins << "]\n{\n";
				m_difficulties[diff].write_chart(outFile);
				outFile << "}\n";
				outFile.flush();
			}
		}
	}
};
