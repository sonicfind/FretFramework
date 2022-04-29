#pragma once
#include <map>
#include "TimedNode.h"
#include "Effect.h"
#include "Midi/MidiFile.h"
#include "../VectorIteration.h"

enum class DifficultyLevel
{
	Expert,
	Hard,
	Medium,
	Easy,
	BRE
};

template <class T>
class NodeTrack
{
	class Difficulty
	{
		friend NodeTrack;

		const char* const m_name;
		std::vector<std::pair<uint32_t, T>> m_notes;
		std::vector<std::pair<uint32_t, std::vector<SustainableEffect*>>> m_effects;
		std::vector<std::pair<uint32_t, std::vector<std::string>>> m_events;

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

		void addEffect(uint32_t position, SustainableEffect* effect)
		{
			VectorIteration::try_emplace(m_effects, position).push_back(effect);
		}

		void addEvent(uint32_t position, const std::string& ev)
		{
			VectorIteration::try_emplace(m_events, position).push_back(ev);
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

	public:
		void load_chart_V1(std::fstream& inFile)
		{
			uint32_t solo = 0;
			std::string line;
			uint32_t prevPosition = 0;
			while (std::getline(inFile, line) && line.find('}') == std::string::npos)
			{
				std::stringstream ss(line);
				uint32_t position;
				ss >> position;

				// Ensure ascending order
				if (position < prevPosition)
					continue;

				prevPosition = position;
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
						addEffect(solo, new Solo(position - solo));
					else
					{
						if (m_events.empty() || m_events.back().first < position)
						{
							static std::pair<uint32_t, std::vector<std::string>> pairNode;
							pairNode.first = position;
							m_events.push_back(pairNode);
						}
						
						m_events.back().second.push_back(std::move(line));
					}
					break;
				case 'n':
				case 'N':
				{
					if (m_notes.empty() || m_notes.back().first < position)
					{
						static std::pair<uint32_t, T> pairNode;
						pairNode.first = position;
						m_notes.push_back(pairNode);
					}

					int lane;
					uint32_t sustain;
					ss >> lane >> sustain;
					m_notes.back().second.initFromChartV1(lane, sustain);
					break;
				}
				case 's':
				case 'S':
				{
					if (m_effects.empty() || m_effects.back().first < position)
					{
						static std::pair<uint32_t, std::vector<SustainableEffect*>> pairNode;
						pairNode.first = position;
						m_effects.push_back(pairNode);
					}

					int type;
					uint32_t duration;
					ss >> type >> duration;
					switch (type)
					{
					case 2:
						addEffect(position, new StarPowerPhrase(duration));
						break;
					case 64:
						addEffect(position, new StarPowerActivation(duration));
						break;
					}
					break;
				}
				}
			}
		}

		void load_cht(std::fstream& inFile)
		{
			std::string line;
			uint32_t prevPosition = 0;
			while (std::getline(inFile, line) && line.find('}') == std::string::npos)
			{
				std::stringstream ss(line);
				uint32_t position;
				ss >> position;

				// Ensure ascending order
				if (position < prevPosition)
					continue;

				prevPosition = position;
				ss.ignore(5, '=');

				char type;
				ss >> type;
				switch (type)
				{
				case 'e':
				case 'E':
					ss.ignore(1);
					std::getline(ss, line);
					if (m_events.empty() || m_events.back().first < position)
					{
						static std::pair<uint32_t, std::vector<std::string>> pairNode;
						pairNode.first = position;
						m_events.push_back(pairNode);
					}

					m_events.back().second.push_back(std::move(line));
					break;
				case 'n':
				case 'N':
				{
					if (m_notes.empty() || m_notes.back().first < position)
					{
						static std::pair<uint32_t, T> pairNode;
						pairNode.first = position;
						m_notes.push_back(pairNode);
					}

					int lane;
					uint32_t sustain = 0;
					ss >> lane >> sustain;

					m_notes.back().second.init(lane, sustain);
				}
				break;
				case 'm':
				case 'M':
					if (!m_notes.empty() && m_notes.back().first == position)
						m_notes.back().second.init_chart2_modifier(ss);
					break;
				case 's':
				case 'S':
				{
					int type = 0;
					uint32_t duration = 0;
					ss >> type >> duration;

					auto check = [&]()
					{
						if (m_effects.empty() || m_effects.back().first < position)
						{
							static std::pair<uint32_t, std::vector<SustainableEffect*>> pairNode;
							pairNode.first = position;
							m_effects.push_back(pairNode);
						}
					};

					switch (type)
					{
					case 2:
						check();
						m_effects.back().second.push_back(new StarPowerPhrase(duration));
						break;
					case 3:
						check();
						m_effects.back().second.push_back(new Solo(duration));
						break;
					case 64:
						check();
						m_effects.back().second.push_back(new StarPowerActivation(duration));
						break;
					case 65:
						check();
						m_effects.back().second.push_back(new Tremolo(duration));
						break;
					case 66:
						check();
						m_effects.back().second.push_back(new Trill(duration));
						break;
					}
				}
				}
			}
		}

		void save_cht(std::fstream& outFile) const
		{
			outFile << "\t[" << m_name << "]\n\t{\n";
			auto noteIter = m_notes.begin();
			auto effectIter = m_effects.begin();
			auto eventIter = m_events.begin();
			bool notesValid = noteIter != m_notes.end();
			bool effectValid = effectIter != m_effects.end();
			bool eventValid = eventIter != m_events.end();

			while (notesValid || effectValid || eventValid)
			{
				while (effectValid &&
					(!notesValid || effectIter->first <= noteIter->first) &&
					(!eventValid || effectIter->first <= eventIter->first))
				{
					for (const auto& eff : effectIter->second)
						eff->save_chart(effectIter->first, outFile);
					effectValid = ++effectIter != m_effects.end();
				}

				while (notesValid &&
					(!effectValid || noteIter->first < effectIter->first) &&
					(!eventValid || noteIter->first <= eventIter->first))
				{
					noteIter->second.save_chart(noteIter->first, outFile);
					notesValid = ++noteIter != m_notes.end();
				}

				while (eventValid &&
					(!effectValid || eventIter->first < effectIter->first) &&
					(!notesValid || eventIter->first < noteIter->first))
				{
					for (const auto& str : eventIter->second)
						outFile << "  " << eventIter->first << " = E " << str << '\n';
					eventValid = ++eventIter != m_events.end();
				}
			}

			outFile << "\t}\n";
			outFile.flush();
		}
	};
	
public:
	const char* const m_name;
	Difficulty m_difficulties[5] = { { "Easy" }, { "Medium" }, { "Hard" }, { "Expert" }, { "BRE" } };

	NodeTrack(const char* name)
		: m_name(name) {}

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

	void load_cht(std::fstream& inFile)
	{
		static char buffer[512] = { 0 };
		while (inFile.getline(buffer, 512) && buffer[0] != '}')
		{
			// Will default to adding to the BRE difficulty if the difficulty name can't be matched
			int i = 0;
			while (i < 4 && !strstr(buffer, m_difficulties[i].m_name))
				++i;

			// Skip '{' line
			inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			m_difficulties[i].load_cht(inFile);
		}
	}

	void load_midi(const unsigned char* currPtr, const unsigned char* const end);

	void save_cht(std::fstream& outFile) const
	{
		if (occupied())
		{
			outFile << "[" << m_name << "]\n{\n";
			for (int diff = 4; diff >= 0; --diff)
				if (m_difficulties[diff].occupied())
					m_difficulties[diff].save_cht(outFile);
			outFile << "}\n";
		}
	}

private:
	void convertNotesToMid(MidiFile::MidiChunk_Track& events) const;

public:

	void save_midi(const char* const name, std::fstream& outFile) const
	{
		MidiFile::MidiChunk_Track events(name);
		for (const auto& vec : m_difficulties[3].m_events)
			for (const auto& ev : vec.second)
				events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MetaEvent_Text(1, ev));

		for (const auto& vec : m_difficulties[3].m_effects)
			for (const auto& effect : vec.second)
				if (effect->getMidiNote() != -1)
				{
					events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote()));
					events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, effect->getMidiNote(), 0));
				}
				else
					for (int lane = 120; lane < 125; ++lane)
					{
						events.addEvent(vec.first, new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane));
						events.addEvent(vec.first + effect->getDuration(), new MidiFile::MidiChunk_Track::MidiEvent_Note(0x90, lane, 0));
					}

		if (hasNotes())
			convertNotesToMid(events);

		events.writeToFile(outFile);
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
void NodeTrack<GuitarNote<5>>::load_midi(const unsigned char* currPtr, const unsigned char* const end);

template<>
void NodeTrack<GuitarNote<6>>::load_midi(const unsigned char* currPtr, const unsigned char* const end);

template<>
void NodeTrack<DrumNote>::load_midi(const unsigned char* currPtr, const unsigned char* const end);

template<>
void NodeTrack<GuitarNote<5>>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const;

template<>
void NodeTrack<GuitarNote<6>>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const;

template<>
void NodeTrack<DrumNote>::convertNotesToMid(MidiFile::MidiChunk_Track& events) const;
