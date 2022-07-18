#include "Song.h"
#include <iostream>

SongAttribute Song::s_currentAttribute = SongAttribute::TITLE;

void Song::scan()
{
	m_ini.load(m_directory);

	if (m_ini.wasLoaded())
	{
		if (m_chartFile.extension() == ".bch")
			scanFile_Bch();
		else if (m_chartFile.extension() == ".cht")
			scanFile_Cht();
		else if (m_chartFile.extension() == ".mid" || m_chartFile.extension() == "midi")
			scanFile_Midi();
		else if (m_chartFile.extension() == ".chart")
			scanFile_Cht();
		else
		{
			m_chartFile.remove_filename();
			throw std::runtime_error(": No valid chart file found in directory");
		}
	}
	else if (m_chartFile.extension() == ".cht" || m_chartFile.extension() == ".chart")
		scanFile_Cht();
	else
		throw std::runtime_error(": Not a valid chart file (possibly just needs a song.ini");

	if (!isValid())
		throw std::runtime_error(": No notes found");

	wait();
}

void Song::scan_full(bool hasIni)
{
	try
	{
		if (hasIni)
		{
			m_ini.load(m_directory);

			if (!m_ini.wasLoaded())
				return;
			
			if (m_chartFile.extension() == ".bch")
				scanFile_Bch();
			else if (m_chartFile.extension() == ".cht")
				scanFile_Cht();
			else if (m_chartFile.extension() == ".mid" || m_chartFile.extension() == "midi")
				scanFile_Midi();
			else
				scanFile_Cht();

			if (!isValid())
			{
				m_hash->interrupt();
				return;
			}
		}
		// It can be assumed that the file has extension .cht or .chart in this 'else' scope
		else
		{
			scanFile_Cht();

			if (!isValid())
			{
				m_hash->interrupt();
				return;
			}

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
			m_ini.save(m_directory);
		}

		finalizeScan();
	}
	catch (std::runtime_error err)
	{
		//std::wcout << m_filepath << ": " << err.what() << '\n';
	}
}

void Song::finalizeScan()
{
	m_last_modified = std::filesystem::last_write_time(m_fullPath);
	if (m_ini.m_song_length == 0)
	{
		std::vector<std::filesystem::path> audioFiles;
		for (const auto& file : std::filesystem::directory_iterator(m_directory))
		{
			if (file.is_regular_file())
			{
				const std::u32string extension = file.path().extension().u32string();
				if (extension == U".ogg" || extension == U".opus" || extension == U".mp3"  || extension == U".wav" || extension == U".flac")
				{
					const std::u32string filename = file.path().stem().u32string();
					if (filename == U"song"   ||
						filename == U"guitar" ||
						filename == U"bass"   ||
						filename == U"rhythm" ||
						filename == U"keys"   ||
						filename == U"vocals_1" || filename == U"vocals_2" ||
						filename == U"drums_1"  || filename == U"drums_2"  || filename == U"drums_3" || filename == U"drums_4")
					{
						// Placeholder for when audio files can be read to retrieve the length
					}
				}
			}
		}
	}
}

bool Song::isValid() const
{
	for (int i = 0; i < 11; ++i)
		if (m_noteTrackScans[i] && m_noteTrackScans[i]->getValue() > 0)
			return true;
	return false;
}
