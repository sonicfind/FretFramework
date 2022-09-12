#include "Song.h"
#include <iostream>

bool Song::scan()
{
	try
	{
		if (m_hasIniFile && !load_Ini(m_directory))
			return false;

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
	if (!m_hasIniFile)
	{
		if (s_noteTracks.drums4_pro.hasNotes())
		{
			setModifier("pro_drums", true);
			if (!s_noteTracks.drums5.hasNotes())
				setModifier("five_lane_drums", false);
		}
		else if (s_noteTracks.drums5.hasNotes())
			setModifier("five_lane_drums", true);
		save_Ini(m_directory);
		m_hasIniFile = true;
	}

	m_last_modified = std::filesystem::last_write_time(m_fullPath);
	if (getSongLength() == 0)
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

constexpr bool Song::validate()
{
	for (int i = 0; i < 11; ++i)
		if (m_noteTrackScans.scanArray[i]->getValue() > 0)
			return true;
	return false;
}

void Song::displayScanResult() const
{
	for (size_t i = 0; i < 11; ++i)
		if (m_noteTrackScans.scanArray[i]->getValue() > 0)
			std::cout << s_noteTracks.trackArray[i]->m_name << ": " << m_noteTrackScans.scanArray[i]->toString() << std::endl;

	m_hash.display();
}
