#pragma once
#include "SongCategory.h"
#include "TaskQueue/TaskQueue.h"

class SongCache
{
	const std::filesystem::path m_location;
	bool m_allowDuplicates = false;

	std::vector<std::unique_ptr<SongEntry>> m_songs;
	std::mutex m_mutex;

	ByTitle       m_category_title;
	ByArtist      m_category_artist;
	ByAlbum       m_category_album;
	ByGenre       m_category_genre;
	ByYear        m_category_year;
	ByCharter     m_category_charter;
	ByPlaylist    m_category_playlist;
	ByArtistAlbum m_category_artistAlbum;

public:
	SongCache(const std::filesystem::path& cacheLocation);
	static void scanDirectory(const std::filesystem::path& directory);

	size_t getNumSongs() const { return m_songs.size(); }
	void toggleDuplicates() { m_allowDuplicates = !m_allowDuplicates; }
	bool areDuplicatesAllowed() const { return m_allowDuplicates; }

	void clear();
	void finalize();

private:
	void validateDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);

	void push(std::unique_ptr<SongEntry>& song);
	void removeDuplicates();
	void fillCategories();
};

extern SongCache g_songCache;
