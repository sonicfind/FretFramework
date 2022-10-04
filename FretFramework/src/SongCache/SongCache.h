#pragma once
#include "SongCategory.h"
#include "TaskQueue/TaskQueue.h"

class SongCache
{
	static std::filesystem::path s_location;
	static bool s_allowDuplicates;

	static std::vector<std::unique_ptr<SongEntry>> s_songs;
	static std::vector<std::filesystem::path> s_directories;
	static std::mutex s_mutex;
	static std::mutex s_directoryMutex;

	static ByTitle       s_category_title;
	static ByArtist      s_category_artist;
	static ByAlbum       s_category_album;
	static ByGenre       s_category_genre;
	static ByYear        s_category_year;
	static ByCharter     s_category_charter;
	static ByPlaylist    s_category_playlist;
	static ByArtistAlbum s_category_artistAlbum;

	static constexpr uint32_t s_CACHE_VERSION = 5;

public:
	static void setLocation(const std::filesystem::path& cacheLocation);

	template <StorageDriveType Drive>
	static void scanDirectory(const std::filesystem::path& directory)
	{
		static const std::filesystem::path NAME_BCH(U"notes.bch");
		static const std::filesystem::path NAME_CHT(U"notes.cht");
		static const std::filesystem::path NAME_MID(U"notes.mid");
		static const std::filesystem::path NAME_MIDI(U"notes.midi");
		static const std::filesystem::path NAME_CHART(U"notes.chart");
		static const std::filesystem::path NAME_INI(U"song.ini");

		if (std::ranges::binary_search(begin(s_directories), end(s_directories), directory))
			return;

		try
		{
			std::pair<bool, std::filesystem::directory_entry> chartFiles[5]{};
			std::pair<bool, std::filesystem::directory_entry> iniFile;

			std::vector<std::filesystem::path> directories;
			for (const auto& file : std::filesystem::directory_iterator(directory))
			{
				if (file.is_directory())
					directories.push_back(file.path());
				else
				{
					const std::filesystem::path filename = file.path().filename();
					if (filename == NAME_CHART)     chartFiles[4] = { true, file };
					else if (filename == NAME_MID)  chartFiles[2] = { true, file };
					else if (filename == NAME_MIDI) chartFiles[3] = { true, file };
					else if (filename == NAME_BCH)  chartFiles[0] = { true, file };
					else if (filename == NAME_CHT)  chartFiles[1] = { true, file };
					else if (filename == NAME_INI)  iniFile = { true, file };
				}
			}

			if (!iniFile.first)
			{
				chartFiles[0].first = false;
				chartFiles[2].first = false;
				chartFiles[3].first = false;
			}

			for (int i = 0; i < 5; ++i)
				if (chartFiles[i].first)
				{
					auto songEntry = std::make_unique<SongEntry>(std::move(chartFiles[i].second), Drive);
					if (iniFile.first)
						if (!songEntry->scan_Ini(iniFile.second) && i != 1 && i != 4)
							return;

					if (songEntry->scan(i))
						push(songEntry);
					return;
				}

			for (auto& subDirectory : directories)
			{
				if constexpr (Drive == SSD)
				{
					TaskQueue::addTask([dir = std::move(subDirectory)]
						{
							scanDirectory<SSD>(dir);
						});
				}
				else
					scanDirectory<HDD>(subDirectory);
			}
		}
		catch (...) {}
	}

	static size_t getNumSongs() { return s_songs.size(); }
	static void toggleDuplicates() { s_allowDuplicates = !s_allowDuplicates; }
	static bool areDuplicatesAllowed() { return s_allowDuplicates; }

	static void clear();
	static void finalize();
	static void displayResultOfFirstSong() { s_songs[0]->displayScanResult(); }
	static void writeCacheFile();
	static void loadCacheFile();

private:
	static void validateDirectory(const std::filesystem::path& directory);
	static bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);

	static void push(std::unique_ptr<SongEntry>& song);
	static void addDirectoryEntry(const std::filesystem::path& directory);
	static void addToCategories(SongEntry* const entry);
};
