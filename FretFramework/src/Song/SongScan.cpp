#include "SongScan.h"
#include <iostream>

SongScan::~SongScan()
{
	for (NoteTrack_Scan* track : m_noteTrackScans)
		if (track)
			delete track;
}

void SongScan::scan(const std::filesystem::path& chartPath)
{
	try
	{
		std::filesystem::path iniPath = m_filepath = chartPath;
		m_ini.load(iniPath.replace_filename("song.ini"));

		if (m_ini.wasLoaded())
		{
			if (m_ini.wasLoaded())
				std::wcout << "Ini file " << iniPath << " loaded" << std::endl;

			if (m_filepath.extension() == ".bch")
				scanFile_Bch();
			else if (m_filepath.extension() == ".cht")
				scanFile_Cht();
			else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
				scanFile_Midi();
			else if (m_filepath.extension() == ".chart")
				scanFile_Cht();
			else
			{
				m_filepath.remove_filename();
				throw std::runtime_error(": No valid chart file found in directory");
			}
		}
		else if (m_filepath.extension() == ".cht" || m_filepath.extension() == ".chart")
			scanFile_Cht();
		else
			throw std::runtime_error(": Not a valid chart file (possibly just needs a song.ini");

		if (!isValid())
			throw std::runtime_error(": No notes found");
	}
	catch (std::runtime_error err)
	{
		std::wcout << err.what() << std::endl;
	}
}

void SongScan::scan_full(const std::filesystem::path& chartPath, const std::filesystem::path& iniPath, const std::vector<std::filesystem::path>& audioFiles)
{
	try
	{
		m_filepath = chartPath;
		if (!iniPath.empty())
			m_ini.load(iniPath);

		if (m_ini.wasLoaded())
		{
			if (m_filepath.extension() == ".bch")
				scanFile_Bch();
			else if (m_filepath.extension() == ".cht")
				scanFile_Cht();
			else if (m_filepath.extension() == ".mid" || m_filepath.extension() == "midi")
				scanFile_Midi();
			else if (m_filepath.extension() == ".chart")
				scanFile_Cht();
		}
		else if (m_filepath.extension() == ".cht" || m_filepath.extension() == ".chart")
			scanFile_Cht();

		if (!isValid())
			return;

		if (!m_ini.wasLoaded())
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
			m_ini.save(m_filepath);
		}

		finalizeScan(audioFiles);
	}
	catch (std::runtime_error err)
	{
		//std::wcout << m_filepath << ": " << err.what() << '\n';
	}
}

void SongScan::finalizeScan(const std::vector<std::filesystem::path>& audioFiles)
{
	m_last_modified = std::filesystem::last_write_time(m_filepath);
	if (m_ini.m_song_length == 0)
	{
		for (const auto& path : audioFiles)
		{
			// Placeholder for when audio files can be read to retrieve the length
		}
	}
}

bool SongScan::isValid() const
{
	for (int i = 0; i < 11; ++i)
		if (m_noteTrackScans[i] && m_noteTrackScans[i]->getValue() > 0)
			return true;
	return false;
}
