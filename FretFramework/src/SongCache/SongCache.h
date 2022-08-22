#pragma once
#include "SongCategory.h"
#include "ThreadedQueue/ThreadedQueue.h"

class ScanQueueNode : public ThreadedQueue::Node
{
	std::filesystem::path m_filepath;
	bool m_hasIni = false;

public:
	ScanQueueNode(std::filesystem::path path, bool ini) : m_filepath(path), m_hasIni(ini) {}
	void process() const noexcept override;
};

class SongCache
{
	const std::filesystem::path m_location;
	bool m_allowDuplicates = false;

	std::vector<std::unique_ptr<Song>> m_songs;
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
	long long scan(const std::vector<std::filesystem::path>& baseDirectories);

	size_t getNumSongs() const { return m_songs.size(); }
	void toggleDuplicates() { m_allowDuplicates = !m_allowDuplicates; }
	bool areDuplicatesAllowed() const { return m_allowDuplicates; }

	void push(std::unique_ptr<Song>& song);

private:
	void stopScan();
	void scanDirectory(const std::filesystem::path& directory);
	bool try_addChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);
	void validateDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);
	

	void clear();
	void finalize();
	void validateSongList();
	void fillCategories();
};

extern SongCache g_songCache;
