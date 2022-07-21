#pragma once
#include "SongCategory.h"
#include "SafeQueue/SafeQueue.h"

class SongCache
{
	const std::filesystem::path m_location;
	bool m_allowDuplicates = false;

	struct ScanQueueNode
	{
		std::unique_ptr<Song> song;
		bool hasIni;
	};

	SafeQueue<ScanQueueNode> m_scanQueue;
	std::vector<std::unique_ptr<Song>> m_songs;

	struct ThreadSet
	{
		enum ThreadStatus
		{
			ACTIVE,
			IDLE,
			STOP,
			QUIT
		};
		std::atomic<ThreadStatus> status = IDLE;

		std::condition_variable idleCondition;
	};
	const unsigned int m_threadCount;
	ThreadSet* m_threadSets;
	std::vector<std::thread> m_threads;

	std::condition_variable m_runningCondition;

	std::mutex m_mutex;
	std::condition_variable m_condition;

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
	void stopThreads();

	void clear();

	void toggleDuplicates() { m_allowDuplicates = !m_allowDuplicates; }
	long long scan(const std::vector<std::filesystem::path>& baseDirectories);
	size_t getNumSongs() const { return m_songs.size(); }
	bool areDuplicatesAllowed() const { return m_allowDuplicates; }

private:
	void startThreads();
	void validateDirectory(const std::filesystem::path& directory);
	void scanDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);
	bool try_addChart(const std::filesystem::path (&chartPaths)[4], bool hasIni);

	void finalize();
	void validateSongList();
	void fillCategories();


	void scanThread(ThreadSet& set);
};
