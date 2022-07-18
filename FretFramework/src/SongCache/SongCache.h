#pragma once
#include "SongCategory.h"
#include "SafeQueue/SafeQueue.h"

class SongCache
{
	struct ScanQueueNode
	{
		Song* song;
		bool hasIni;
	};

	struct ThreadSet
	{
		SafeQueue<ScanQueueNode> queue;

		std::mutex mutex;
		std::condition_variable condition;
	};
	std::vector<Song*> m_songs;

	std::mutex m_sharedMutex;
	std::condition_variable m_sharedCondition;
	typename std::list<ThreadSet>::iterator m_setIter;
	std::list<ThreadSet> m_sets;
	std::vector<std::thread> m_threads;

	enum 
	{
		WAITING_FOR_EXIT,
		EXIT
	} m_status = WAITING_FOR_EXIT;

	const std::filesystem::path m_location;
	bool m_allowDuplicates = false;


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
	~SongCache();
	void startThreads();
	void stopThreads();

	void clear();

	void toggleDuplicates() { m_allowDuplicates = !m_allowDuplicates; }
	long long scan(const std::vector<std::filesystem::path>& baseDirectories);
	size_t getNumSongs() const { return m_songs.size(); }
	bool areDuplicatesAllowed() const { return m_allowDuplicates; }

private:
	void validateDirectory(const std::filesystem::path& directory);
	void scanDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);
	bool try_addChart(const std::filesystem::path (&chartPaths)[4], bool hasIni);

	void finalize();
	void validateSongList();
	void fillCategories();

	void runScanner(ThreadSet& set);
};
