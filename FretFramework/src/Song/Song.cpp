#include "Song.h"
#include "FileChecks/FilestreamCheck.h"
#include <iostream>

Song::Tracks Song::s_noteTracks;
SongEntry Song::s_baseEntry;


void Song::newSong()
{
	s_baseEntry = SongEntry();
	m_currentSongEntry = &s_baseEntry;
	reset();
}

void Song::loadFrom(const std::filesystem::path& chartPath)
{
	s_baseEntry = SongEntry(chartPath);
	s_baseEntry.load_Ini();
	m_currentSongEntry = &s_baseEntry;
	load();
	s_baseEntry.setBaseModifiers();
}

void Song::loadFrom(SongEntry* const entry)
{
	m_currentSongEntry = entry;
	load();
}

void Song::load()
{
	reset();

	const FilePointers file(m_currentSongEntry->getFilePath());
	const auto ext = m_currentSongEntry->getChartFile().extension();
	if (ext == ".chart" || ext == ".cht")
		loadFile(TextTraversal(file));
	else if (ext == ".mid" || ext == "midi")
		loadFile(MidiTraversal(file));
	else
		loadFile(BCHTraversal(file));
}

void Song::save()
{
	try
	{
		m_currentSongEntry->setModifier("lyrics", s_noteTracks.vocals.hasNotes() || s_noteTracks.harmonies.hasNotes());
		m_currentSongEntry->removeModifier("star_power_note");

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
				if (const bool fiveLaneOccipied = s_noteTracks.drums5.occupied(); fiveLaneOccipied || !s_noteTracks.drums4_pro.occupied())
				{
					m_currentSongEntry->removeModifier("pro_drums");

					if (fiveLaneOccipied)
						m_currentSongEntry->setModifier("five_lane_drums", true);
					else
						m_currentSongEntry->removeModifier("five_lane_drums");
				}
				else
				{
					m_currentSongEntry->setModifier("pro_drums", true);
					m_currentSongEntry->setModifier("five_lane_drums", false);
				}

				m_currentSongEntry->setChartFile(U"notes.mid.test");
				saveFile_Midi();
				loop = false;
			}
			else if (answer == 'c' || answer == 'b')
			{
				if (s_noteTracks.drums4_pro.hasNotes())
				{
					m_currentSongEntry->setModifier("pro_drums", true);
					if (s_noteTracks.drums5.hasNotes())
						m_currentSongEntry->removeModifier("five_lane_drums");
					else
						m_currentSongEntry->setModifier("five_lane_drums", false);
				}
				else
				{
					m_currentSongEntry->removeModifier("pro_drums");

					if (s_noteTracks.drums5.hasNotes())
						m_currentSongEntry->setModifier("five_lane_drums", true);
					else
						m_currentSongEntry->removeModifier("five_lane_drums");
				}

				if (answer == 'c')
				{
					m_currentSongEntry->setChartFile(U"notes.cht");
					saveFile_Cht();
				}
				else
				{
					m_currentSongEntry->setChartFile(U"notes.bch");
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

		m_currentSongEntry->save_Ini();
	}
	catch (FilestreamCheck::InvalidFileException e)
	{

	}
}

void Song::reset()
{
	clearTracks();

	m_tickrate = 192;

	m_sync.clear();
	m_sync.push_back({ 0, SyncValues(true, true) });
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

	for (NoteTrack* const track : s_noteTracks.trackArray)
		track->adjustTicks(multiplier);
}
