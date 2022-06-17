#include "SongCache.h"

SongCache::SongCache(const std::filesystem::path& cacheLocation)
	: m_location(cacheLocation)
	, m_thread(&SongCache::scanThread, this) {}

SongCache::~SongCache()
{
	g_scanStatus = EXIT;
	m_conditions[0].notify_one();
	m_thread.join();
}

void SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	m_songlist.clear();
	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (const std::filesystem::path& directory : baseDirectories)
			scanDirectory(directory);

	std::unique_lock lk(m_mutex);
	while (!m_scanQueue.empty())
		m_conditions[0].wait(lk);

	for (auto iter = m_songlist.begin(); iter != m_songlist.end();)
		if (iter->isValid())
		{
			//std::cout << iter->getFilepath() << " - ";
			//iter->displayHash();
			++iter;
		}
		else
			m_songlist.erase(iter++);

	Traversal::waitForHashThread();
}

void SongCache::scanDirectory(const std::filesystem::path& directory)
{
	std::vector<std::filesystem::directory_entry> directories;
	std::vector<std::filesystem::path> audioFiles;
	std::filesystem::path iniPath;

	// In order of precendence
	// .bch
	// .cht
	// .mid
	// .chart
	std::filesystem::path chartPaths[4];

	for (const auto& file : std::filesystem::directory_iterator(directory))
	{
		if (file.is_directory())
			directories.push_back(file);
		else
		{
			const std::filesystem::path filename = file.path().filename();
			if (filename == "notes.bch")
				chartPaths[0] = file.path();
			else if (filename == "notes.cht")
				chartPaths[1] = file.path();
			else if (filename == "notes.mid")
				chartPaths[2] = file.path();
			else if (filename == "notes.chart")
				chartPaths[3] = file.path();
			else if (filename == "song.ini")
				iniPath = file.path();
			else if ((filename.extension() == ".ogg" || filename.extension() == ".wav" || filename.extension() == ".mp3" || filename.extension() == ".opus" || filename.extension() == ".flac") &&
				(filename.stem() == "song" ||
					filename.stem() == "guitar" ||
					filename.stem() == "bass" ||
					filename.stem() == "rhythm" ||
					filename.stem() == "keys" ||
					filename.stem() == "vocals_1" || filename.stem() == "vocals_2" ||
					filename.stem() == "drums_1" || filename.stem() == "drums_2" || filename.stem() == "drums_3" || filename.stem() == "drums_4"))
				audioFiles.push_back(file.path());
		}
	}

	if (!try_addChart(chartPaths, iniPath, audioFiles))
		for (const auto& dir : directories)
			scanDirectory(dir);
}

bool SongCache::try_addChart(const std::filesystem::path (&chartPaths)[4], const std::filesystem::path& iniPath, const std::vector<std::filesystem::path>& audioFiles)
{
	for (int i = 0; i < 4; ++i)
		if (!chartPaths[i].empty() && (!iniPath.empty() || i & 1))
		{
			std::unique_lock lk(m_mutex);
			while (g_scanStatus == USING_QUEUE)
				m_conditions[1].wait(lk);

			m_scanQueue.emplace_back(m_songlist.emplace_back(), chartPaths[i], iniPath, audioFiles);
			m_conditions[0].notify_one();
			return true;
		}
	return false;
}

void SongCache::scanThread()
{
	std::unique_lock lk(m_mutex);
	while (true)
	{
		while (g_scanStatus != EXIT && m_scanQueue.empty())
			m_conditions[0].wait(lk);

		if (g_scanStatus == EXIT)
			break;

		g_scanStatus = USING_QUEUE;
		SongScan scan = m_scanQueue.front();
		m_scanQueue.pop_front();
		g_scanStatus = WAITING;
		m_conditions[1].notify_one();

		scan.song.scan_full(scan.chartPath, scan.iniPath, scan.audioFiles);
		m_conditions[0].notify_one();
	}
}
