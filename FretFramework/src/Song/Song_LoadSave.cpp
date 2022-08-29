#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::load()
{
	m_sync = { {0, SyncValues(true, true)} };
	m_ini.load(m_directory);

	const FilePointers file(m_fullPath);
	const auto ext = m_chartFile.extension();
	if (ext == ".chart" || ext == ".cht")
		loadFile(TextTraversal(file));
	else if (ext == ".mid" || ext == "midi")
		loadFile(MidiTraversal(file));
	else if (ext == ".bch")
		loadFile(BCHTraversal(file));
	else
		throw InvalidFileException(m_fullPath.string());
	m_version_cht = 2;
	m_version_bch = 1;
}

void Song::save()
{
	try
	{
		if (s_noteTracks[9]->hasNotes() || s_noteTracks[10]->hasNotes())
			m_ini.m_lyrics = true;

		bool loop = true;
		do
		{
			char answer = -1;
			std::cout << "Valid Options for Export: C - .cht | M - .mid | B - .bch\n";
			std::cout << "Q/q - Do not save\n";
			std::cout << "Select Chart Type: ";
			std::cin >> answer;
			std::cin.clear();
			answer = std::tolower(answer);
			if (answer == 'q')
				return;

			m_ini.m_lyrics = s_noteTracks[9]->hasNotes() || s_noteTracks[10]->hasNotes();
			if (answer == 'm')
			{
				m_ini.m_multiplier_note = 116;
				m_ini.m_star_power_note = 116;

				bool useFiveLane = false;
				if (s_noteTracks[7]->occupied() && s_noteTracks[8]->occupied())
				{
					char answer = -1;
					loop = true;
					do
					{
						std::cout << "Select Drum Track to save: 4 or 5?\n";
						std::cout << "Q/q - Do not save file\n";
						std::cout << "Answer: ";
						std::cin >> answer;
						std::cin.clear();
						switch (answer)
						{
						case '5':
							useFiveLane = true;
							__fallthrough;
						case '4':
							loop = false;
							break;
						case 'q':
						case 'Q':
							return;
						}
					} while (loop);
				}
				else
					useFiveLane = s_noteTracks[8]->occupied();

				if (useFiveLane || !s_noteTracks[7]->occupied())
				{
					m_ini.m_pro_drums.deactivate();
					m_ini.m_pro_drum.deactivate();

					if (useFiveLane)
						m_ini.m_five_lane_drums = true;
					else
						m_ini.m_five_lane_drums.deactivate();
				}
				else
				{
					m_ini.m_pro_drums = true;
					m_ini.m_pro_drum = true;
					m_ini.m_five_lane_drums = false;
				}

				setChartFile(U"notes.mid.test");
				saveFile_Midi();
				loop = false;
			}
			else if (answer == 'c' || answer == 'b')
			{
				m_ini.m_multiplier_note = 0;
				m_ini.m_star_power_note = 0;

				if (s_noteTracks[7]->hasNotes())
				{
					m_ini.m_pro_drums = true;
					m_ini.m_pro_drum = true;
					if (s_noteTracks[8]->hasNotes())
						m_ini.m_five_lane_drums.deactivate();
					else
						m_ini.m_five_lane_drums = false;
				}
				else
				{
					m_ini.m_pro_drums.deactivate();
					m_ini.m_pro_drum.deactivate();

					if (s_noteTracks[8]->hasNotes())
						m_ini.m_five_lane_drums = true;
					else
						m_ini.m_five_lane_drums.deactivate();
				}

				if (answer == 'c')
				{
					setChartFile(U"notes.cht");
					saveFile_Cht();
				}
				else
				{
					setChartFile(U"notes.bch");
					saveFile_Bch();
				}
				loop = false;
			}
			else
			{
				std::string tmp;
				std::getline(std::cin, tmp);
			}
		} while (loop);

		m_ini.save(m_directory);
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}


void Song::setTickRate(uint16_t tickRate)
{
	float multiplier = float(tickRate) / m_tickrate;
	m_tickrate = tickRate;
	for (auto& sync : m_sync)
		sync.first = uint32_t(sync.first * multiplier);
	for (auto& sect : m_sectionMarkers)
		sect.first = uint32_t(sect.first * multiplier);
	for (auto& vec : m_globalEvents)
		vec.first = uint32_t(vec.first * multiplier);

	Sustainable::multiplyThresholds(multiplier);

	for (auto& track : s_noteTracks)
		track->adjustTicks(multiplier);
}
