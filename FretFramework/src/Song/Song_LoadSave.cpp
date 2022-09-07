#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

void Song::load()
{
	m_sync.clear();
	m_sync.push_back({ 0, SyncValues(true, true) });
	m_hasIniFile = load_Ini(m_directory);

	{
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
	}

	m_version_bch = 1;
}

void Song::save()
{
	try
	{
		setModifier("lyrics", s_noteTracks[9]->hasNotes() || s_noteTracks[10]->hasNotes());
		removeModifier("star_power_note");

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

			if (answer == 'm')
			{
				if (const bool fiveLaneOccipied = s_noteTracks[8]->occupied(); fiveLaneOccipied || !s_noteTracks[7]->occupied())
				{
					removeModifier("pro_drums");

					if (fiveLaneOccipied)
						setModifier("five_lane_drums", true);
					else
						removeModifier("five_lane_drums");
				}
				else
				{
					setModifier("pro_drums", true);
					setModifier("five_lane_drums", false);
				}

				setChartFile(U"notes.mid.test");
				saveFile_Midi();
				loop = false;
			}
			else if (answer == 'c' || answer == 'b')
			{
				if (s_noteTracks[7]->hasNotes())
				{
					setModifier("pro_drums", true);
					if (s_noteTracks[8]->hasNotes())
						removeModifier("five_lane_drums");
					else
						setModifier("five_lane_drums", false);
				}
				else
				{
					removeModifier("pro_drums");

					if (s_noteTracks[8]->hasNotes())
						setModifier("five_lane_drums", true);
					else
						removeModifier("five_lane_drums");
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

		save_Ini(m_directory);
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
