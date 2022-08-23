#include "Song.h"
#include <iostream>

SongAttribute Song::s_sortAttribute = SongAttribute::TITLE;

bool Song::scan(bool hasIni)
{
	try
	{
		if (hasIni)
		{
			m_ini.load(m_directory);
			if (!m_ini.wasLoaded())
				return false;
		}

		const FilePointers file(m_fullPath);
		const auto ext = m_chartFile.extension();
		if (ext == U".cht" || ext == U".chart")
			scanFile(TextTraversal(file));
		else if (ext == U".mid" || ext == U"midi")
			scanFile(MidiTraversal(file));
		else
			scanFile(BCHTraversal(file));

		if (!validate())
			return false;

		m_hash.computeHash(file);
	}
	catch (std::runtime_error err)
	{
		//std::wcout << m_filepath << ": " << err.what() << '\n';
		return false;
	}
	return true;
}

void Song::finalizeScan()
{
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
		m_ini.save(m_directory);
	}

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

bool Song::validate()
{
	bool valid = false;
	for (int i = 0; i < 11; ++i)
		if (m_noteTrackScans[i])
		{
			if (m_noteTrackScans[i]->getValue() > 0)
				valid = true;
			else
				m_noteTrackScans[i].reset();
		}
	return valid;
}

void Song::displayScanResult() const
{
	for (size_t i = 0; i < 11; ++i)
		if (m_noteTrackScans[i])
			std::cout << s_noteTracks[i]->m_name << ": " << m_noteTrackScans[i]->toString() << std::endl;

	m_hash.display();
}
