#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

Song::Song()
	: m_sync({ {0, SyncValues(true, true)} }) {}

Song::Song(const std::filesystem::path& filepath)
	: Song()
{
	load(filepath);
}

Song::~Song()
{
	for (NoteTrack* track : s_noteTracks)
		track->clear();
}

void Song::load(const std::filesystem::path& filepath)
{
	std::filesystem::path iniPath = m_filepath = filepath;
	m_ini.load(iniPath.replace_filename("song.ini"));

	if (m_filepath.extension() == ".chart" || m_filepath.extension() == ".cht")
		loadFile_Cht();
	else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
		loadFile_Midi();
	else if (m_filepath.extension() == ".bch")
		loadFile_Bch();
	else
		throw InvalidFileException(m_filepath.string());
	m_version_cht = 2;
	m_version_bch = 1;
}

void Song::save()
{
	try
	{
		std::filesystem::path outPath = m_filepath;

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

				outPath.replace_extension(".mid.test");
				saveFile_Midi(outPath);
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
					outPath.replace_extension(".cht");
					saveFile_Cht(outPath);
				}
				else
				{
					outPath.replace_extension(".bch");
					saveFile_Bch(outPath);
				}
				loop = false;
			}
		} while (loop);

		m_ini.save(outPath);
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}

void Song::setFilepath(const std::filesystem::path& filename)
{
	m_filepath = filename;
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

	// Sets the threshold default for forcing guitar notes and for sustains
	// Automatically sets each threshold to 1/3 of the tickrate if they are at their default values
	m_ini.m_hopo_frequency *= multiplier;
	Sustainable::setForceThreshold(m_ini.m_hopo_frequency);
	m_ini.m_sustain_cutoff_threshold *= multiplier;
	Sustainable::setsustainThreshold(m_ini.m_sustain_cutoff_threshold);

	for (auto& track : s_noteTracks)
		track->adjustTicks(multiplier);
}
