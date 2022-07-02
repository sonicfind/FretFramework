#pragma once
#include "Song/Song.h"
#include "SafeQueue/SafeQueue.h"

class SongCache
{
	struct ScanQueueNode
	{
		Song& song;
		bool hasIni;
	};

	struct ThreadSet
	{
		SafeQueue<ScanQueueNode> queue;

		std::mutex mutex;
		std::condition_variable condition;
	};

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
	std::list<Song> m_songlist;
	bool m_allowDuplicates = false;

public:
	SongCache(const std::filesystem::path& cacheLocation);
	~SongCache();
	void toggleDuplicates() { m_allowDuplicates = !m_allowDuplicates; }
	void scan(const std::vector<std::filesystem::path>& baseDirectories);
	size_t getNumSongs() const { return m_songlist.size(); }
	bool areDuplicatesAllowed() const { return m_allowDuplicates; }

private:
	void validateDirectory(const std::filesystem::path& directory);
	void scanDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], bool hasIni);
	bool try_addChart(const std::filesystem::path (&chartPaths)[4], bool hasIni);

	void finalize();
	void validateSongList();
	void validateSongList_allowDuplicates();

	void runScanner(ThreadSet& set);
};
