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
		std::map<uint32_t, Fret> m_soloes;
		std::map<uint32_t, std::vector<std::string>> m_events;
		
		void load_chart(std::fstream& inFile, const bool version2)
		{
			uint32_t solo = 0;
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
					if (line == "solo")
						solo = position;
					else if (line == "soloend")
						m_soloes[solo].init(position - solo);
					else
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

		void save_chart(std::fstream& outFile) const
		{
			std::map<uint32_t, std::vector<std::string>> soloEvents;
			for (const auto& solo : m_soloes)
			{
				soloEvents[solo.first].push_back("solo");
				soloEvents[solo.first + solo.second.getSustain()].push_back("soloend");
			}

			auto noteIter = m_notes.begin();
			auto starIter = m_starPower.begin();
			auto eventIter = m_events.begin();
			auto soloIter = soloEvents.begin();
			bool notesValid = noteIter != m_notes.end();
			bool starValid = starIter != m_starPower.end();
			bool eventValid = eventIter != m_events.end();
			bool soloValid = soloIter != soloEvents.end();
			
			while (soloValid || notesValid || starValid || eventValid)
			{
				while (soloValid &&
					(!notesValid || soloIter->first <= noteIter->first) &&
					(!eventValid || soloIter->first <= eventIter->first) &&
					(!starValid || soloIter->first <= starIter->first))
				{
					for (const auto& str : soloIter->second)
						outFile << "  " << soloIter->first << " = E " << str << '\n';
					soloValid = ++soloIter != soloEvents.end();
				}

				while (notesValid &&
					(!soloValid || noteIter->first < soloIter->first) &&
					(!eventValid || noteIter->first <= eventIter->first) &&
					(!starValid || noteIter->first <= starIter->first))
				{
					noteIter->second.save_chart(noteIter->first, outFile);
					notesValid = ++noteIter != m_notes.end();
				}

				while (starValid &&
					(!notesValid || starIter->first < noteIter->first) &&
					(!soloValid || starIter->first < soloIter->first) &&
					(!eventValid || starIter->first <= eventIter->first))
				{
					starIter->second.save_chart(starIter->first, 2, outFile, 'S');
					starValid = ++starIter != m_starPower.end();
				}

				while (eventValid &&
					(!notesValid || eventIter->first < noteIter->first) &&
					(!soloValid || eventIter->first < soloIter->first) &&
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

	void load_chart(std::string& line, std::fstream& inFile, const bool version2)
	{
		if (line.find("Expert") != std::string::npos)
			m_difficulties[0].load_chart(inFile, version2);
		else if (line.find("Hard") != std::string::npos)
			m_difficulties[1].load_chart(inFile, version2);
		else if (line.find("Medium") != std::string::npos)
			m_difficulties[2].load_chart(inFile, version2);
		else if (line.find("Easy") != std::string::npos)
			m_difficulties[3].load_chart(inFile, version2);
		else
		{
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '}');
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	void save_chart(const std::string_view& ins, std::fstream& outFile) const
	{
		static std::string_view difficultyStrings[] = { "Expert", "Hard", "Medium", "Easy" };
		for (int diff = 0; diff < 4; ++diff)
		{
			if (m_difficulties[diff])
			{
				outFile << "[" << difficultyStrings[diff] << ins << "]\n{\n";
				m_difficulties[diff].save_chart(outFile);
				outFile << "}\n";
				outFile.flush();
			}
		}
	}
};
