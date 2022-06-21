#include "SongCache.h"
#include <set>

SongCache::SongCache(const std::filesystem::path& cacheLocation)
	: m_location(cacheLocation)
{
	unsigned int numThreads = std::thread::hardware_concurrency() >= 4 ? std::thread::hardware_concurrency() / 2 : 1;
	for (unsigned int i = 0; i < numThreads; ++i)
		m_threads.emplace_back(std::thread(&SongCache::runScanner, this, std::ref(m_sets.emplace_back())));
	m_setIter = m_sets.begin();
}

SongCache::~SongCache()
{
	m_status = EXIT;
	for (auto& set : m_sets)
		set.condition.notify_one();

	for (auto& thr : m_threads)
		thr.join();
}

void SongCache::scan(const std::vector<std::filesystem::path>& baseDirectories)
{
	m_songlist.clear();
	auto t1 = std::chrono::high_resolution_clock::now();
	if (m_location.empty() || !std::filesystem::exists(m_location))
		for (const std::filesystem::path& directory : baseDirectories)
			scanDirectory(directory);
	auto t2 = std::chrono::high_resolution_clock::now();
	long long count = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "Directory search took " << count / 1000 << " milliseconds\n";
	finalize();
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
			else if ((filename.extension() == ".ogg" || filename.extension() == ".mp3" || filename.extension() == ".opus" || filename.extension() == ".wav"   || filename.extension() == ".flac") &&
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
			auto iter = m_setIter++;
			if (m_setIter == m_sets.end())
				m_setIter = m_sets.begin();

			iter->queue.push({ m_songlist.emplace_back(), chartPaths[i], iniPath, audioFiles });
			iter->condition.notify_one();
			return true;
		}
	return false;
}

void SongCache::finalize()
{
	std::unique_lock lk(m_sharedMutex);
	for (auto& set : m_sets)
		m_sharedCondition.wait(lk, [&] { return set.queue.empty(); });

	if (!m_allowDuplicates)
		SongBase::waitForHasher();

	std::set<MD5> finalSetList;

	for (auto iter = m_songlist.begin(); iter != m_songlist.end();)
		if (iter->isValid() && (m_allowDuplicates || !finalSetList.contains(iter->getHash())))
		{
			if (!m_allowDuplicates)
				finalSetList.insert(iter->getHash());
			++iter;
		}
		else
		{
			/*if (!iter->isValid())
				std::cout << "No notes: " << iter->getPath() << '\n';
			else
				std::cout << "Duplicate: " << iter->getPath() << '\n';*/
			m_songlist.erase(iter++);
		}

	if (m_allowDuplicates)
		SongBase::waitForHasher();
}

void SongCache::runScanner(ThreadSet& set)
{
	std::unique_lock lk(set.mutex);
	do
	{
		while (!set.queue.empty())
		{
			ScanQueueNode& scan = set.queue.front();
			scan.song.scan_full(scan.chartPath, scan.iniPath, scan.audioFiles);
			set.queue.pop();
		}
		m_sharedCondition.notify_one();
		set.condition.wait(lk);
	} while (m_status != EXIT);
}
