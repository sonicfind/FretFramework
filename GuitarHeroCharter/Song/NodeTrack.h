#pragma once
#include <map>
#include <vector>
#include "TimedNode.h"
#include "Effect.h"

enum class DifficultyLevel
{
	Expert,
	Hard,
	Medium,
	Easy,
	BRE
};

template <typename N>
class MidiTrackFiller;

template <class T>
class NodeTrack
{
	class Difficulty
	{
		friend NodeTrack;
		friend class MidiTrackFiller<T>;

		std::map<uint32_t, T> m_notes;
		std::map<uint32_t, std::vector<SustainableEffect*>> m_effects;
		std::map<uint32_t, uint32_t> m_soloes;
		std::map<uint32_t, std::vector<std::string>> m_events;

		void addNote(int note, uint32_t position, uint32_t sustain = 0)
		{
			m_notes[position].init(note, sustain);
		}

		void addNoteFromMid(int note, uint32_t position, uint32_t sustain = 0)
		{
			if (sustain < 20)
				sustain = 0;
			m_notes[position].initFromMid(note, sustain);
		}

		void addEffect(uint32_t position, SustainableEffect* effect)
		{
			m_effects[position].push_back(effect);
		}

		void addSolo(uint32_t position, uint32_t sustain)
		{
			m_soloes[position] = sustain;
		}

		void addEvent(uint32_t position, const std::string& ev)
		{
			m_events[position].push_back(ev);
		}

		bool modifyNote(uint32_t position, char modifier, bool toggle = true)
		{
			return m_notes[position].modify(modifier, toggle);
		}

		bool modifyColor(int note, uint32_t position, char modifier)
		{
			return m_notes[position].modifyColor(note, modifier);
		}

		uint32_t getNumActiveColors(uint32_t position)
		{
			try
			{
				return m_notes.at(position).getNumActiveColors();
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
		bool occupied() const { return !m_notes.empty() || !m_events.empty() || !m_soloes.empty() || !m_effects.empty(); }
		~Difficulty()
		{
			for (auto& vec : m_effects)
				for (auto& eff : vec.second)
					delete eff;
		}

	public:
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
				switch (type)
				{
				case 'e':
				case 'E':
					ss.ignore(1);
					std::getline(ss, line);
					if (line == "solo")
						solo = position;
					else if (line == "soloend")
						m_soloes[solo] = position - solo;
					else
						m_events[position].push_back(std::move(line));
					break;
				case 'n':
				case 'N':
				{
					size_t lane;
					uint32_t sustain;
					ss >> lane >> sustain;

					if (!version2)
						m_notes[position].initFromChartV1(lane, sustain);
					else
						m_notes[position].init(lane, sustain);
				}
					break;
				case 'm':
				case 'M':
					m_notes[position].init_chart2_modifier(ss);
					break;
				default:
				{
					uint32_t duration;
					switch (type)
					{
					case 's':
					case 'S':
					{
						int type;
						ss >> type >> duration;
						if (ss)
						{
							switch (type)
							{
							case 2:
								m_effects[position].push_back(new StarPowerPhrase(duration));
								break;
							case 64:
								m_effects[position].push_back(new StarPowerActivation(duration));
								break;
							}
						}
						break;
					}
					case 'r':
					case 'R':
						m_effects[position].push_back(new Tremolo(duration));
						break;
					case 'd':
					case 'D':
						m_effects[position].push_back(new Trill(duration));
						break;
					}
				}
				}
			}
		}

		void save_chart(std::fstream& outFile) const
		{
			std::map<uint32_t, std::vector<std::string>> soloEvents;
			for (const auto& solo : m_soloes)
			{
				soloEvents[solo.first].push_back(" = E solo\n");
				soloEvents[solo.first + solo.second].push_back(" = E soloend\n");
			}

			auto noteIter = m_notes.begin();
			auto effectIter = m_effects.begin();
			auto eventIter = m_events.begin();
			auto soloIter = soloEvents.begin();
			bool notesValid = noteIter != m_notes.end();
			bool effectValid = effectIter != m_effects.end();
			bool eventValid = eventIter != m_events.end();
			bool soloValid = soloIter != soloEvents.end();

			while (soloValid || notesValid || effectValid || eventValid)
			{
				while (soloValid &&
					(!effectValid || soloIter->first <= effectIter->first) &&
					(!notesValid || soloIter->first <= noteIter->first) &&
					(!eventValid || soloIter->first <= eventIter->first))
				{
					for (const auto& str : soloIter->second)
						outFile << "  " << soloIter->first << str;
					soloValid = ++soloIter != soloEvents.end();
				}

				while (effectValid &&
					(!soloValid || effectIter->first < soloIter->first) &&
					(!notesValid || effectIter->first <= noteIter->first) &&
					(!eventValid || effectIter->first <= eventIter->first))
				{
					for (const auto& eff : effectIter->second)
						eff->save_chart(effectIter->first, outFile);
					effectValid = ++effectIter != m_effects.end();
				}

				while (notesValid &&
					(!soloValid || noteIter->first < soloIter->first) &&
					(!effectValid || noteIter->first < effectIter->first) &&
					(!eventValid || noteIter->first <= eventIter->first))
				{
					noteIter->second.save_chart(noteIter->first, outFile);
					notesValid = ++noteIter != m_notes.end();
				}

				while (eventValid &&
					(!soloValid || eventIter->first < soloIter->first) &&
					(!effectValid || eventIter->first < effectIter->first) &&
					(!notesValid || eventIter->first < noteIter->first))
				{
					for (const auto& str : eventIter->second)
						outFile << "  " << eventIter->first << " = E " << str << '\n';
					eventValid = ++eventIter != m_events.end();
				}
			}
		}
	};
	
public:
	Difficulty m_difficulties[5];

	Difficulty& operator[](size_t i)
	{
		if (i >= 5)
			throw std::out_of_range("Max difficulty index is 4");
		return m_difficulties[i];
	}

	const Difficulty& operator[](size_t i) const
	{
		if (i >= 5)
			throw std::out_of_range("Max difficulty index is 4");
		return m_difficulties[i];
	}

	void load_midi(std::fstream& inFile, const std::streampos& end);

	void save_chart(const std::string_view& ins, std::fstream& outFile) const
	{
		static std::string_view difficultyStrings[] = { "Expert", "Hard", "Medium", "Easy" };
		for (int diff = 0; diff < 4; ++diff)
		{
			if (m_difficulties[diff].occupied())
			{
				outFile << "[" << difficultyStrings[diff] << ins << "]\n{\n";
				m_difficulties[diff].save_chart(outFile);
				outFile << "}\n";
				outFile.flush();
			}
		}
	}
	
	// Returns whether any difficulty in this track contains notes
	// ONLY checks for notes
	bool hasNotes() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.hasNotes())
				return true;
		return false;
	}

	// Returns whether any difficulty in this track contains notes, effects, soloes, or other events
	bool occupied() const
	{
		for (const auto& diff : m_difficulties)
			if (diff.occupied())
				return true;
		return false;
	}
};

template<>
void NodeTrack<GuitarNote<5>>::load_midi(std::fstream& inFile, const std::streampos& end);

template<>
void NodeTrack<GuitarNote<6>>::load_midi(std::fstream& inFile, const std::streampos& end);

template<>
void NodeTrack<DrumNote>::load_midi(std::fstream& inFile, const std::streampos& end);
