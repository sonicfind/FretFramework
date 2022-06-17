#pragma once
#include "Song/Song.h"
class SongCache
{

	struct SongScan
	{
		Song& song;
		std::filesystem::path chartPath;
		std::filesystem::path iniPath;
		std::vector<std::filesystem::path> audioFiles;
	};
	std::list<SongScan> m_scanQueue;

	std::mutex m_mutex;
	std::condition_variable m_conditions[2];
	std::thread m_thread;

	enum scanStatus
	{
		WAITING,
		USING_QUEUE,
		EXIT
	} g_scanStatus = scanStatus::WAITING;

	const std::filesystem::path m_location;
	std::list<Song> m_songlist;

public:
	SongCache(const std::filesystem::path& cacheLocation);
	~SongCache();
	void scan(const std::vector<std::filesystem::path>& baseDirectories);
	size_t getNumSongs() const { return m_songlist.size(); }

private:
	void validateDirectory(const std::filesystem::path& directory);
	void scanDirectory(const std::filesystem::path& directory);
	bool try_validateChart(const std::filesystem::path(&chartPaths)[4], const std::filesystem::path& iniPath, const std::vector<std::filesystem::path>& audioFiles);
	bool try_addChart(const std::filesystem::path (&chartPaths)[4], const std::filesystem::path& iniPath, const std::vector<std::filesystem::path>& audioFiles);
	void scanThread();
};

