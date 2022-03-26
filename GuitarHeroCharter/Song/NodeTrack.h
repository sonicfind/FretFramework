#pragma once
#include "TimedNode.h"
#include "Effect.h"
#include "Midi/FileHandler/MidiFile.h"
#include <map>
#include <vector>
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
	struct Difficulty
	{
		std::map<uint32_t, T> m_notes;
		std::map<uint32_t, std::vector<Effect*>> m_effects;
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
				switch (type)
				{
				case 'e':
				case 'E':
					ss.ignore(1);
					std::getline(ss, line);
					if (line == "solo")
						solo = position;
					else if (line == "soloend")
						m_soloes[solo].init(position - solo);
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
					ss >> duration;
					if (ss)
					{
						switch (type)
						{
						case 's':
						case 'S':
							m_effects[position].push_back(new StarPowerPhrase(duration));
							break;
						case 'a':
						case 'A':
							m_effects[position].push_back(new StarPowerActivation(duration));
							break;
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
					(!notesValid || soloIter->first <= noteIter->first) &&
					(!eventValid || soloIter->first <= eventIter->first) &&
					(!effectValid || soloIter->first <= effectIter->first))
				{
					for (const auto& str : soloIter->second)
						outFile << "  " << soloIter->first << " = E " << str << '\n';
					soloValid = ++soloIter != soloEvents.end();
				}

				while (notesValid &&
					(!soloValid || noteIter->first < soloIter->first) &&
					(!eventValid || noteIter->first <= eventIter->first) &&
					(!effectValid || noteIter->first <= effectIter->first))
				{
					noteIter->second.save_chart(noteIter->first, outFile);
					notesValid = ++noteIter != m_notes.end();
				}

				while (effectValid &&
					(!notesValid || effectIter->first < noteIter->first) &&
					(!soloValid || effectIter->first < soloIter->first) &&
					(!eventValid || effectIter->first <= eventIter->first))
				{
					for (const auto& eff : effectIter->second)
						eff->save_chart(effectIter->first, outFile);
					effectValid = ++effectIter != m_effects.end();
				}

				while (eventValid &&
					(!notesValid || eventIter->first < noteIter->first) &&
					(!soloValid || eventIter->first < soloIter->first) &&
					(!effectValid || eventIter->first < effectIter->first))
				{
					for (const auto& str : eventIter->second)
						outFile << "  " << eventIter->first << " = E " << str << '\n';
					eventValid = ++eventIter != m_events.end();
				}
			}
		}

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

		void addEffect(uint32_t position, Effect* effect)
		{
			m_effects[position].push_back(effect);
		}

		void addSolo(uint32_t position, uint32_t sustain = 0)
		{
			m_soloes[position].init(sustain);
		}

		void addEvent(uint32_t position, std::string& ev)
		{
			m_events[position].push_back(std::move(ev));
		}

		bool modifyNote(uint32_t position, char modifier, bool toggle = true)
		{
			return m_notes[position].modify(modifier, toggle);
		}

		bool modifyColor(int note, uint32_t position, char modifier)
		{
			return m_notes[position].modifyColor(note, modifier);
		}

		operator bool() const { return m_notes.size(); }
		~Difficulty()
		{
			for (auto& vec : m_effects)
				for (auto& eff : vec.second)
					delete eff;
		}
	};
	Difficulty m_difficulties[5];
	std::map<uint32_t, std::vector<std::string>> m_trackEvents;
	
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

	void load_midi(const MidiFile::MidiChunk_Track& track)
	{
		struct
		{
			bool greenToOpen = false;
			bool sliderNotes = false;
			uint32_t notes[9] = { 0 };
			uint32_t flam = 0;
		} difficultyTracker[5];
		// Diff 5 = BRE

		uint32_t solo = 0;
		uint32_t starPower = 0;
		uint32_t fill = 0;
		uint32_t tremolo = 0;
		uint32_t trill = 0;

		for (auto& vec : track.m_events)
		{
			for (auto ptr : vec.second)
			{
				switch(ptr->m_syntax)
				{
				case 0xF0:
				{
					MidiFile::MidiChunk_Track::SysexEvent* sysex = static_cast<MidiFile::MidiChunk_Track::SysexEvent*>(ptr);
					if (sysex->m_data[4] == 255)
					{
						switch (sysex->m_data[5])
						{
						case 1:
							for (auto& diff : difficultyTracker)
								diff.greenToOpen = sysex->m_data[6];
							break;
						case 4:
							for (auto& diff : difficultyTracker)
								diff.sliderNotes = sysex->m_data[6];
							break;
						}
					}
					else
					{
						switch (sysex->m_data[5])
						{
						case 1:
							difficultyTracker[3 - sysex->m_data[4]].greenToOpen = sysex->m_data[6];
							break;
						case 4:
							difficultyTracker[3 - sysex->m_data[4]].sliderNotes = sysex->m_data[6];
							break;
						}
					}
					break;
				}
				case 0xFF:
				{
					MidiFile::MidiChunk_Track::MetaEvent* ev = static_cast<MidiFile::MidiChunk_Track::MetaEvent*>(ptr);
					if (ev->m_type < 16)
					{
						MidiFile::MidiChunk_Track::MetaEvent_Text* text = static_cast<MidiFile::MidiChunk_Track::MetaEvent_Text*>(ptr);
						if (text->m_type == 0x01)
							m_trackEvents.at(vec.first).emplace_back(text->m_text);
						// Currently not used
						// Requires adding vocal track support
						else if (text->m_type == 0x05){}
					}
					break;
				}
				case 0x90:
				case 0x80:
				{
					MidiFile::MidiChunk_Track::MidiEvent_Note* note = static_cast<MidiFile::MidiChunk_Track::MidiEvent_Note*>(ptr);
					/*
					* Special values:
					* 
					*	95 (drums) = Expert+ Double Bass
					*	103 = Solo
					*	105/106 = vocals
					*	108 = pro guitar
					*	109 = Drum flam
					*	115 = Pro guitar solo
					*	116 = star power/overdrive
					*	126 = tremolo
					*	127 = trill
					*/

					// Soloes
					if (note->m_note == 103)
					{
						if (note->m_syntax == 0x90 && note->m_velocity == 100)
							solo = vec.first;
						else
							m_difficulties[0].addSolo(solo, vec.first - solo);
					}
					// Star Power
					else if (note->m_note == 116)
					{
						if (note->m_syntax == 0x90 && note->m_velocity == 100)
							starPower = vec.first;
						else
							m_difficulties[0].addEffect(vec.first, new StarPowerPhrase(vec.first - starPower));
					}
					// Tremolo (or single drum roll)
					else if (note->m_note == 126)
					{
						if (note->m_syntax == 0x90 && note->m_velocity == 100)
							tremolo = vec.first;
						else
							m_difficulties[0].addEffect(vec.first, new Tremolo(vec.first - tremolo));
					}
					// Trill (or special drum roll)
					else if (note->m_note == 127)
					{
						if (note->m_syntax == 0x90 && note->m_velocity == 100)
							trill = vec.first;
						else
							m_difficulties[0].addEffect(vec.first, new Trill(vec.first - trill));
					}
					// Drum-specifics
					else if (T::isDrums())
					{
						static bool toms[3] = { false };
						// Expert+
						if (note->m_note == 95)
						{
							if (note->m_syntax == 0x90 && note->m_velocity == 100)
								difficultyTracker[0].notes[0] = vec.first;
							else
								m_difficulties[0].modifyColor(0, difficultyTracker[0].notes[0], 'X');
						}
						// Notes
						else if (60 <= note->m_note && note->m_note < 102)
						{
							int noteValue = note->m_note - 60;
							int diff = 3 - (noteValue / 12);
							int lane = noteValue % 12;
							if (lane < 9)
							{
								if (note->m_syntax == 0x90 && note->m_velocity == 100)
								{
									difficultyTracker[diff].notes[lane] = vec.first;

									if (2 <= lane && lane < 5 && toms[lane - 2])
										m_difficulties[diff].modifyColor(lane, difficultyTracker[diff].notes[lane], 'T');

								}
								else
								{
									m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], vec.first - difficultyTracker[diff].notes[lane]);
								}
							}
						}
						// Tom markers
						else if (110 <= note->m_note && note->m_note <= 112)
							toms[note->m_note - 110] = note->m_syntax == 0x90 && note->m_velocity > 0;
						// Flams
						else if (note->m_note == 109)
						{
							if (note->m_syntax == 0x90 && note->m_velocity == 100)
								difficultyTracker[0].flam = vec.first;
							else
								m_difficulties[0].modifyNote(difficultyTracker[0].flam, 'F');
						}
						// Fill
						else if (note->m_note == 125)
						{
							if (note->m_syntax == 0x90 && note->m_velocity == 100)
								fill = vec.first;
							else
								m_difficulties[0].addEffect(vec.first, new StarPowerActivation(vec.first - fill));
						}
					}
					// Notes
					else if (58 <= note->m_note && note->m_note < 103)
					{
						int noteValue = note->m_note - 58;
						int diff = 3 - (noteValue / 12);
						int lane = noteValue % 12;

						if (T::isGHL())
						{
							if (1 <= lane && lane < 4)
								lane += 3;
							else if (4 <= lane && lane < 7)
								lane -= 3;
						}
						else if (difficultyTracker[diff].greenToOpen && lane == 2)
							// 0 = open
							lane = 0;
						else
							// Only by one since Green starts with index 1
							--lane;

						if (note->m_syntax == 0x90 && note->m_velocity == 100)
							difficultyTracker[diff].notes[lane] = vec.first;
						else
						{
							m_difficulties[diff].addNoteFromMid(lane, difficultyTracker[diff].notes[lane], vec.first - difficultyTracker[diff].notes[lane]);
							if (difficultyTracker[diff].sliderNotes)
								m_difficulties[diff].modifyNote(difficultyTracker[diff].notes[lane], 'T', false);
						}
					}
					break;
				}
				}
			}
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
